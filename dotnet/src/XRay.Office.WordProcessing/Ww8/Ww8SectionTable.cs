using System.Buffers.Binary;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Ww8;

/// <summary>
/// Reads a DOC's section descriptors: where each section ends, and how its pages are set up.
/// </summary>
/// <remarks>
/// <para>
/// Two levels of indirection, both easy to get wrong. <c>PlcfSed</c> is a PLCF in the table stream
/// whose twelve-byte records each hold an offset — but the offset points into the <em>WordDocument</em>
/// stream, not the table stream it was read from, and it points at a length-prefixed grpprl rather than
/// at a structure. So a reader has to change streams halfway through and then decode sprms.
/// </para>
/// <para>
/// The section properties themselves are sprms, decoded by the same machinery as paragraph and
/// character properties, with the ids verified against LibreOffice's own
/// <c>sw/source/filter/ww8/sprmids.hxx</c>. Two of them are stated in a way that invites an off-by-one:
/// <c>sprmSCcolumns</c> holds the column count <em>minus one</em>, so a value of zero means one column;
/// and <c>sprmSBOrientation</c> uses Word's <c>DM_*</c> numbering where 1 is portrait and 2 is
/// landscape, so a plain truth test makes every portrait page landscape.
/// </para>
/// </remarks>
internal static class Ww8SectionTable
{
    /// <summary>The size of an <c>SED</c>, the PLCF's data record.</summary>
    /// <remarks>
    /// Passed explicitly, as every PLCF's must be: read as though it had no data records, this PLCF
    /// takes its record bytes for character positions and puts section boundaries at random offsets.
    /// </remarks>
    internal const int DescriptorSize = 12;

    /// <summary>Where in an <c>SED</c> the offset of the section's properties sits.</summary>
    private const int PropertiesOffsetAt = 2;

    /// <summary>
    /// An offset of all-ones means the section has no properties of its own.
    /// </summary>
    /// <remarks>
    /// Written by producers rather than merely theoretical, and it is not a small number — following
    /// it reads whatever happens to be at the end of the stream.
    /// </remarks>
    private const uint NoProperties = 0xFFFFFFFF;

    /// <summary>How many sections are read before the rest are ignored.</summary>
    /// <remarks>
    /// A guard on untrusted input, not a real limit: a long report has tens of sections and a
    /// generated one may claim thousands.
    /// </remarks>
    private const int MaxSections = 4096;

    /// <summary>
    /// Reads every section's geometry, in document order.
    /// </summary>
    /// <param name="descriptors">The <c>PlcfSed</c>, already parsed with the right record size.</param>
    /// <param name="wordDocument">The <c>WordDocument</c> stream, where the property sets live.</param>
    internal static List<WritingSection> Read(Ww8Plcf descriptors, ReadOnlyMemory<byte> wordDocument)
    {
        List<WritingSection> sections = [];

        int count = Math.Min(descriptors.Count, MaxSections);
        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> descriptor = descriptors.Record(i);
            if (descriptor.Length < DescriptorSize)
            {
                // A truncated descriptor still delimits a section, so it gets the defaults rather
                // than being dropped — losing a section would shift every later one.
                sections.Add(new WritingSection());
                continue;
            }

            uint at = BinaryPrimitives.ReadUInt32LittleEndian(descriptor[PropertiesOffsetAt..]);
            sections.Add(ReadProperties(PropertiesAt(wordDocument, at)));
        }

        ResolveContinuousBreaks(sections);
        return sections;
    }

    /// <summary>
    /// Settles what a continuous section break does to the page, which is much less than it says.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A DOC section descriptor always restates the whole page setup, whatever kind of break it opens with,
    /// so a continuous section carries a full set of margins that Word never applies: a break that does not
    /// start a fresh sheet cannot re-cut the sheet it lands on. Reading the restated values as the page's is
    /// what makes a document paginate against a page it never has.
    /// </para>
    /// <para>
    /// LibreOffice draws the line in <c>wwSectionManager::InsertSegments</c>
    /// (<c>sw/source/filter/ww8/ww8par.cxx</c>:4422). A section that is continuous <em>and</em> agrees with
    /// the one before it about the sheet — width, height and orientation — becomes a Writer
    /// <em>text</em> section rather than a page descriptor, and a text section carries only columns and a
    /// left/right indent (<c>InsertSection</c>, <c>ww8par6.cxx</c>:717). Everything the page descriptor
    /// holds — the sheet, the head and foot bands, the vertical margins — stays as the last section that
    /// made one left it, which is what this copies forward.
    /// </para>
    /// <para>
    /// The other half of the same rule is the sentence that comment opens with: <em>"If two following
    /// sections are different in following properties, Word will interpret a continuous section break
    /// between them as if it was a section break next page."</em> An incompatible continuous section does
    /// get its own page descriptor, and giving a node a page descriptor in Writer starts a page — so the
    /// break is promoted rather than honoured. <b>That promotion is the whole of what this does.</b>
    /// </para>
    /// <para>
    /// <strong>Carrying the sheet and the vertical margins forward is <see cref="Layout.ContinuousPageDescriptors"/>'s
    /// job and not this one</strong>, and doing it here as well was wrong rather than merely redundant.
    /// A continuous section that states furniture of its own <em>does</em> get a descriptor — hung on the
    /// first hard page break inside it, which is the <em>"nightmare scenario"</em> arm of
    /// <c>InsertSegments</c> — and that descriptor carries the section's own vertical margins.
    /// <c>Resolve</c> already declines to inherit in exactly that case, and this pass had overwritten the
    /// margins before it could.
    /// </para>
    /// <para>
    /// Measured on <c>words/done-014/doc/PK_FlugzeugeStricken.doc</c>, whose second section is
    /// continuous, states its own running head and states <c>sprmSDyaTop</c> = 1701 twips against the
    /// first section's 1417. Patching that 1701 to 2500 moves the reference's body from 85.08 pt to
    /// 125.03 and repaginates it from 7 pages to 11, and moved <b>nothing at all</b> here: the squash had
    /// already replaced it with 1417, and our body sat at 70.88 pt on every one of pages 3 to 7 — 14.20 pt
    /// too high, which is two extra lines by page 6.
    /// </para>
    /// <para>
    /// Measured on <c>foca_form_1.doc</c>, whose second section is continuous, begins inside the opening
    /// table, and states a top margin of 1135 twips against the first section's 567. Taking it at its word
    /// started the body an inch down every page and pushed the paragraph after the table onto a sheet of
    /// its own: four pages against the reference's three.
    /// </para>
    /// </remarks>
    private static void ResolveContinuousBreaks(List<WritingSection> sections)
    {
        for (int i = 1; i < sections.Count; i++)
        {
            WritingSection section = sections[i];
            if (section.Break != SectionBreak.Continuous) continue;

            // The section before it as this pass has already settled it, so a run of continuous sections
            // all inherit from the last one that cut a page rather than from each other's restatements.
            PageGeometry carried = sections[i - 1].Page;

            if (!SameSheet(section.Page, carried))
            {
                sections[i] = section with { Break = SectionBreak.NextPage };
            }
        }
    }

    /// <summary>
    /// True when two sections would print on the same sheet, which is what decides whether a continuous
    /// break can be honoured at all.
    /// </summary>
    /// <remarks>
    /// The three properties LibreOffice compares, and only those: a section may change its margins, its
    /// columns and its running heads across a continuous break, but not its paper.
    /// </remarks>
    private static bool SameSheet(PageGeometry section, PageGeometry previous)
        => section.Size.Width == previous.Size.Width
            && section.Size.Height == previous.Size.Height
            && section.IsLandscape == previous.IsLandscape;

    /// <summary>
    /// The grpprl a descriptor's offset names, or empty when there is none.
    /// </summary>
    /// <remarks>
    /// The two-byte length prefix counts the grpprl's bytes and not itself, so the properties start
    /// two bytes further on. Both the offset and the length come from the file, so both are checked
    /// against the stream rather than trusted.
    /// </remarks>
    private static ReadOnlyMemory<byte> PropertiesAt(ReadOnlyMemory<byte> wordDocument, uint offset)
    {
        if (offset == NoProperties || offset + 2 > (uint)wordDocument.Length) return default;

        int at = (int)offset;
        int length = BinaryPrimitives.ReadUInt16LittleEndian(wordDocument.Span[at..]);
        int start = at + 2;

        if (length <= 0 || start >= wordDocument.Length) return default;
        return wordDocument.Slice(start, Math.Min(length, wordDocument.Length - start));
    }

    /// <summary>
    /// What a <c>sprmSBkc</c> value means.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word's own <c>bkc</c> numbering, from [MS-DOC] 2.9.4: 0 continuous, 1 new column, 2 new page,
    /// 3 even page, 4 odd page. LibreOffice reads it the same way — <c>wwSection::IsContinuous()</c> in
    /// <c>sw/source/filter/ww8/ww8par.hxx</c> is <c>maSep.bkc == 0</c>, and <c>InsertSegments()</c> takes
    /// the column-break branch only for <c>bkc == 1</c>.
    /// </para>
    /// <para>
    /// The two easily swap, and swapping them is expensive rather than cosmetic: a continuous break is by
    /// far the commonest kind — it is what a document uses to change column count or margins mid-page —
    /// and reading it as a column break puts a page break in the middle of every one of them, which then
    /// shifts every page after it.
    /// </para>
    /// </remarks>
    private static SectionBreak BreakOf(int bkc) => bkc switch
    {
        0 => SectionBreak.Continuous,
        1 => SectionBreak.NewColumn,
        3 => SectionBreak.EvenPage,
        4 => SectionBreak.OddPage,

        // Two means a new page, and so does anything a file states that Word does not define: the
        // default a section descriptor with no sprmSBkc at all takes is 2 (`ww8scan.cxx`, WW8_SEP's
        // constructor).
        _ => SectionBreak.NextPage,
    };

    /// <summary>
    /// Where a header sits when the section states no <c>sprmSDyaHdrTop</c>: half an inch down.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Half an inch, not nothing. WW8's <c>SEP</c> is a structure with defaults rather than a set of
    /// statements, and Word writes a sprm only where the section differs from them — so a document that
    /// never mentions the header distance is asking for 720 twips, and reading the silence as zero puts
    /// every header 36 pt above where it belongs and every footer 36 pt below.
    /// </para>
    /// <para>
    /// The same 720 appears twice in LibreOffice's own reader: in <c>WW8_SEP</c>'s constructor
    /// (<c>ww8scan.cxx</c>, <c>dyaHdrTop(720), dyaHdrBottom(720)</c>) and again as the fallback passed to
    /// <c>ReadUSprm</c> when the section table is walked (<c>ww8par6.cxx:1183</c>).
    /// </para>
    /// <para>
    /// It reaches the body as well as the furniture, because <see cref="PageGeometry.HeaderHeight"/> is
    /// the gap between this and the top margin: a wrong distance makes the header's band the wrong height
    /// too, so a header of more than one line overflows into the text rather than pushing it down.
    /// </para>
    /// </remarks>
    private static readonly Length DefaultHeaderDistance = Length.FromTwips(720);

    /// <summary>
    /// What a section that never states <c>sprmSDxaColumns</c> means by it: 1.25 cm.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not zero. <c>WW8_SEP</c>'s own constructor and the fallback passed to <c>ReadUSprm</c> when the
    /// section table is walked both say 708 twips — <c>aNewSection.maSep.dxaColumns = ReadUSprm(pSep,
    /// pIds[4], 708)</c> (<c>ww8par6.cxx</c>:987), commented "default distance 1.25 cm". The same figure
    /// reaches DOCX through <c>SectionPropertyMap</c>'s <c>m_nColumnDistance(1249)</c>, 12.49 mm.
    /// </para>
    /// <para>
    /// It is not only the gutter between columns that a zero gets wrong: the column *width* is the text
    /// width less the gaps, divided by the count, so a two-column section on a 504 pt measure comes out
    /// 252 pt wide instead of 234 pt — 8% too wide, so every line of it breaks late and the section is
    /// short of the lines the reference gives it.
    /// </para>
    /// </remarks>
    private static readonly Length DefaultColumnGap = Length.FromTwips(708);

    internal static WritingSection ReadProperties(ReadOnlyMemory<byte> grpprl)
    {
        PageGeometry page = PageGeometry.Default;
        PageMargins margins = PageMargins.Default;
        DocSize size = page.Size;

        SectionBreak sectionBreak = SectionBreak.NextPage;
        Length gutter = Length.Zero;
        Length headerDistance = DefaultHeaderDistance;
        Length footerDistance = DefaultHeaderDistance;
        Length columnGap = DefaultColumnGap;
        int columns = 1;

        // sprmSFEvenlySpaced defaults to *on*, which is what `SwWW8ImplReader::SetCols` reads
        // (`ww8par6.cxx`:996, `ReadBSprm(..., 1)`): a section that says nothing about its columns' widths
        // wants them equal. The per-column widths and spacings are collected regardless and only used when
        // the flag says they are meant to be, since a stale set left over from an earlier edit is common.
        bool evenlySpaced = true;
        SortedDictionary<int, Length> columnWidths = [];
        SortedDictionary<int, Length> columnSpacings = [];

        // The section's four BRCs and the packed sprmSPgbProp that says what to do with them. Kept as
        // the decoded structures rather than as a PageBorders because the WW9 form supersedes the WW8
        // one side by side, and a section can state both.
        Ww8Border?[] borderSides = new Ww8Border?[4];
        int pageBorderProperties = 0;

        bool landscape = false;
        bool rightToLeft = false;
        bool titlePage = false;
        int? restartAt = null;
        bool restartsNumbering = false;
        NoteNumberFormat pageNumberFormat = NoteNumberFormat.Arabic;

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                case Ww8SprmReader.Ids.SectionBreakKind:
                    sectionBreak = BreakOf(sprm.Byte);
                    break;

                case Sprms.PageWidth:
                    if (Dimension(sprm) is { } width) size = size with { Width = width };
                    break;
                case Sprms.PageHeight:
                    if (Dimension(sprm) is { } height) size = size with { Height = height };
                    break;

                case Sprms.LeftMargin:
                    margins = margins with { Left = Twips(sprm) };
                    break;
                case Sprms.RightMargin:
                    margins = margins with { Right = Twips(sprm) };
                    break;
                case Sprms.TopMargin:
                    margins = margins with { Top = Twips(sprm) };
                    break;
                case Sprms.BottomMargin:
                    margins = margins with { Bottom = Twips(sprm) };
                    break;

                case Sprms.Gutter:
                    gutter = Twips(sprm);
                    break;
                case Sprms.HeaderDistance:
                    headerDistance = Twips(sprm);
                    break;
                case Sprms.FooterDistance:
                    footerDistance = Twips(sprm);
                    break;

                case Sprms.ColumnCount:
                    // Minus one, as the name in [MS-DOC] says: ccolM1. Zero is one column.
                    columns = Math.Clamp(sprm.Word + 1, 1, 64);
                    break;
                case Sprms.ColumnGap:
                    columnGap = Twips(sprm);
                    break;

                case Sprms.EvenlySpacedColumns:
                    evenlySpaced = sprm.Byte != 0;
                    break;

                // Three bytes, and the first of them is the column the other two are about: Word writes one
                // of these per column rather than an array, so they arrive in no guaranteed order and a
                // section can state a width for a column it does not have. Keyed by the stated index and
                // sorted, which reads a well-formed section exactly and cannot mis-order a malformed one.
                case Sprms.ColumnWidth:
                    if (IndexedTwips(sprm) is { } stated) columnWidths[stated.At] = stated.Length;
                    break;
                case Sprms.ColumnSpacing:
                    if (IndexedTwips(sprm) is { } spacing) columnSpacings[spacing.At] = spacing.Length;
                    break;

                // sprmSFBiDi, one byte: the section reads right to left, which reverses the order
                // of its columns and nothing else.
                case Sprms.RightToLeft:
                    rightToLeft = sprm.Byte != 0;
                    break;

                case Sprms.Orientation:
                    // Word's DM_PORTRAIT is 1 and DM_LANDSCAPE is 2, so the test is against 2 rather
                    // than against zero.
                    landscape = sprm.Byte == 2;
                    break;

                case Sprms.TitlePage:
                    titlePage = sprm.Byte != 0;
                    break;

                // The page border, in either of its two forms. A WW9 BRC that is present overrides the
                // WW8 one for the same side — `lcl_ReadBorders` reads the four 80s and then the four 9s
                // over the top of them (`ww8par6.cxx`:1345-1364) — which falls out of taking them in
                // sprm order only because Word writes the 80s first; assigning per side keeps it true
                // whichever order they arrive in, since the 9 form is the one with the RGB colour.
                case Sprms.BorderTop80:
                case Sprms.BorderLeft80:
                case Sprms.BorderBottom80:
                case Sprms.BorderRight80:
                    Assign(borderSides, sprm.Identifier - Sprms.BorderTop80, sprm, isVersion9: false);
                    break;

                case Sprms.BorderTop:
                case Sprms.BorderLeft:
                case Sprms.BorderBottom:
                case Sprms.BorderRight:
                    Assign(borderSides, sprm.Identifier - Sprms.BorderTop, sprm, isVersion9: true);
                    break;

                case Sprms.PageBorderProperties:
                    pageBorderProperties = sprm.Word;
                    break;

                case Sprms.RestartsPageNumbering:
                    restartsNumbering = sprm.Byte != 0;
                    break;
                case Sprms.PageNumberStart97:
                    restartAt = sprm.Word;
                    break;
                case Sprms.PageNumberStart:
                    restartAt = sprm.DoubleWord;
                    break;

                case Sprms.PageNumberFormat:
                    // sep.nfcPgn: 0 arabic, 1 I II III, 2 i ii iii, 3 A B C, 4 a b c. LibreOffice's
                    // own table (`wwSectionManager::SetNumberingType`, ww8par6.cxx:841) is exactly this,
                    // and it clamps anything above 4 to arabic rather than rejecting the section.
                    pageNumberFormat = sprm.Byte switch
                    {
                        1 => NoteNumberFormat.UpperRoman,
                        2 => NoteNumberFormat.LowerRoman,
                        3 => NoteNumberFormat.UpperLetter,
                        4 => NoteNumberFormat.LowerLetter,
                        _ => NoteNumberFormat.Arabic,
                    };

                    break;

                default:
                    break;
            }
        }

        return new WritingSection
        {
            Page = new PageGeometry
            {
                Size = size,
                Margins = margins,
                Gutter = gutter,
                HeaderDistance = headerDistance,
                FooterDistance = footerDistance,

                // As in DOCX: sprmSDyaTop is the body's top margin and sprmSDyaHdrTop the header's
                // distance from the page edge, so the header's own height is the gap between them.
                HeaderHeight = Gap(headerDistance, margins.Top),
                FooterHeight = Gap(footerDistance, margins.Bottom),
                Columns = columns,
                ColumnGap = columnGap,
                ColumnRuler = Ruler(columns, evenlySpaced, columnWidths, columnSpacings),
                IsLandscape = landscape,
                IsRightToLeft = rightToLeft,
                Borders = Borders(borderSides, pageBorderProperties),
            },

            Break = sectionBreak,

            // The start value only applies when the section says it restarts. A document that carries
            // a stale start value from an earlier edit is common, and honouring it renumbers pages
            // that Word numbers continuously.
            RestartPageNumberAt = restartsNumbering ? restartAt ?? 1 : null,
            PageNumberFormat = pageNumberFormat,
            HasDifferentFirstPage = titlePage,
        };
    }

    /// <summary>Records one side's <c>BRC</c>, in top, left, bottom, right order.</summary>
    private static void Assign(Ww8Border?[] sides, int at, Ww8Sprm sprm, bool isVersion9)
    {
        if (at is < 0 or > 3) return;

        // A WW9 sprm's operand is variable-length and begins with a count byte in some producers'
        // files; `SetWW8_BRC` simply reinterprets from the start, so this does too.
        if (Ww8DocumentReader.ReadBorder(sprm.Operand.Span, isVersion9) is { } border)
        {
            sides[at] = border;
        }
    }

    /// <summary>
    /// The page border the section's four <c>BRC</c>s and <c>sprmSPgbProp</c> come to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>sprmSPgbProp</c> is three fields packed into one word, and only two of them do anything:
    /// <c>pgbApplyTo</c> in bits 0-2, <c>pgbPageDepth</c> in bits 3-4 and <c>pgbOffsetFrom</c> in bits
    /// 5-7 (<c>ww8par6.cxx</c>:1195-1199). <c>pgbOffsetFrom == 1</c> is "from the edge of the page",
    /// which is the *default* spelling in DOCX and the non-default one here.
    /// </para>
    /// <para>
    /// <c>pgbApplyTo</c> is read exactly as <c>wwSectionManager::InsertSegments</c> reads it, which is
    /// as two independent flags rather than as an enumeration: <c>!(pgbApplyTo &amp; 1)</c> gives the
    /// border to the section's ordinary pages and <c>!(pgbApplyTo &amp; 2)</c> to its first
    /// (<c>ww8par.cxx</c>:4303-4306). So 0 is every page, 1 the first page, 2 every page but the first
    /// — and <strong>3, which [MS-DOC] calls "the whole document", draws nothing at all</strong>,
    /// because both flags are set. That is reproduced rather than corrected: it is what the reference
    /// renders.
    /// </para>
    /// <para>
    /// <c>pgbPageDepth</c> — in front of or behind the text — is <c>w:zOrder</c> by another name and
    /// is ignored here for the same reason it is ignored in the DOCX reader: LibreOffice reads it into
    /// <c>maSep.pgbPageDepth</c> and never uses the field, so honouring it would move us away from the
    /// reference.
    /// </para>
    /// </remarks>
    private static PageBorders? Borders(Ww8Border?[] sides, int properties)
    {
        int applyTo = properties & 0x0007;
        if (applyTo is 3 or > 4) return null;

        Ww8Border? right = sides[3];

        PageBorders borders = new()
        {
            Top = sides[0]?.ResolvedPageSide ?? default,
            Left = sides[1]?.ResolvedPageSide ?? default,
            Bottom = sides[2]?.ResolvedPageSide ?? default,
            Right = right?.ResolvedPageSide ?? default,
            OffsetFromText = ((properties & 0x00E0) >> 5) != 1,

            // The shadow is the right side's, with Word's own floor of sixteen twips — 0.8 pt — under
            // it: `SetShadow` raises anything below 0x10 to 0x10 (`ww8par6.cxx`:1548-1562), which the
            // OOXML path has no equivalent of.
            Shadow = right is { HasShadow: true } shadowed && shadowed.Width > Length.Zero
                ? Length.Max(shadowed.Width, Length.FromTwips(16))
                : Length.Zero,

            Display = applyTo switch
            {
                1 => PageBorderDisplay.FirstPage,
                2 => PageBorderDisplay.NotFirstPage,
                _ => PageBorderDisplay.AllPages,
            },
        };

        return borders.Draws ? borders : null;
    }

    /// <summary>
    /// The room left between the furniture's edge and the body's, never negative.
    /// </summary>
    /// <remarks>
    /// A header distance beyond the top margin is Word being told to let the header overlap the body,
    /// which means the header has no reserved height rather than a negative one.
    /// </remarks>
    private static Length Gap(Length furnitureEdge, Length bodyEdge)
    {
        Length gap = bodyEdge - furnitureEdge;
        return gap > Length.Zero ? gap : Length.Zero;
    }

    /// <summary>A page dimension, or null when it is absent or implausible.</summary>
    /// <remarks>
    /// Fitted to the nearest standard paper dimension when it is within 0.44 mm of one, which is
    /// what <c>ww8par6.cxx</c>:521 and :1083 do to <c>sprmSXaPage</c> and <c>sprmSYaPage</c> and to
    /// no other section measurement — see <see cref="Model.PaperSizes"/>.
    /// </remarks>
    private static Length? Dimension(Ww8Sprm sprm)
    {
        int twips = sprm.Word;
        return twips is > 0 and <= 22 * 1440
            ? Model.PaperSizes.SloppyFit(Length.FromTwips(twips))
            : null;
    }

    /// <summary>
    /// A measurement in twips, signed.
    /// </summary>
    /// <remarks>
    /// Signed because a negative top margin is how a document lets a header hang above the page's own
    /// edge, and reading it unsigned turns minus a centimetre into most of a metre.
    /// </remarks>
    private static Length Twips(Ww8Sprm sprm) => Length.FromTwips(sprm.SignedWord);

    /// <summary>
    /// A three-byte operand that names a column and then measures it.
    /// </summary>
    /// <remarks>
    /// <c>sprmSDxaColWidth</c> and <c>sprmSDxaColSpacing</c> are the only section sprms shaped this way,
    /// and both are read exactly as <c>SwWW8ImplReader::SetCols</c> reads them: the index in the first
    /// byte and an unsigned width in the two after it (<c>ww8par6.cxx</c>:1006-1030, which reaches for
    /// them through <c>HasSprm(id, nColumn)</c> — a lookup by that first byte).
    /// </remarks>
    private static (int At, Length Length)? IndexedTwips(Ww8Sprm sprm)
    {
        if (sprm.Operand.Length < 3) return null;

        ReadOnlySpan<byte> operand = sprm.Operand.Span;
        int at = operand[0];

        return at is >= 0 and < MaxColumns
            ? (at, Length.FromTwips(BinaryPrimitives.ReadUInt16LittleEndian(operand[1..])))
            : null;
    }

    /// <summary>
    /// The stated column widths, when the section says its columns are not evenly spaced and states
    /// enough of them to be believed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Null unless every column has a width, which is the leniency this needs: Word writes one sprm per
    /// column and a truncated or edited section can be missing some, in which case the count and the gap
    /// still describe the section perfectly well and a half-filled ruler would not. LibreOffice's own
    /// reader substitutes 1440 twips for a missing width — a plain inch, unrelated to the measure — which
    /// is a worse answer than even columns for every page size but one.
    /// </para>
    /// <para>
    /// The last column has no spacing after it, so a section states one fewer of those than of the widths.
    /// </para>
    /// </remarks>
    private static ColumnRuler? Ruler(
        int columns,
        bool evenlySpaced,
        SortedDictionary<int, Length> widths,
        SortedDictionary<int, Length> spacings)
    {
        if (evenlySpaced || columns < 2) return null;

        List<Length> stated = new(columns);
        List<Length> gaps = new(columns - 1);

        for (int i = 0; i < columns; i++)
        {
            if (!widths.TryGetValue(i, out Length width) || width <= Length.Zero) return null;
            stated.Add(width);

            if (i < columns - 1) gaps.Add(spacings.GetValueOrDefault(i));
        }

        return new ColumnRuler(stated, gaps);
    }

    /// <summary>The most columns a section can have, which is what <c>ccolM1</c> is clamped to above.</summary>
    private const int MaxColumns = 64;

    /// <summary>
    /// The section sprms, from LibreOffice's <c>sprmids.hxx</c>.
    /// </summary>
    /// <remarks>
    /// Taken from the C++ tree rather than from the specification's prose, because the header states
    /// each id's operand size in the same line and that is the half that decides whether the following
    /// sprm decodes at all.
    /// </remarks>
    private static class Sprms
    {
        internal const ushort TitlePage = 0x300A;
        internal const ushort ColumnCount = 0x500B;
        internal const ushort ColumnGap = 0x900C;

        /// <summary><c>sprmSDxaColWidth</c>: one column's own width, three bytes, index first.</summary>
        internal const ushort ColumnWidth = 0xF203;

        /// <summary><c>sprmSDxaColSpacing</c>: the gap after one column, in the same shape.</summary>
        internal const ushort ColumnSpacing = 0xF204;

        /// <summary><c>sprmSFEvenlySpaced</c>: off when the section states a width per column.</summary>
        internal const ushort EvenlySpacedColumns = 0x3005;
        internal const ushort RestartsPageNumbering = 0x3011;

        /// <summary><c>sprmSNfcPgn</c>, the sequence the section's page numbers are written in.</summary>
        internal const ushort PageNumberFormat = 0x300E;
        internal const ushort HeaderDistance = 0xB017;
        internal const ushort FooterDistance = 0xB018;
        internal const ushort PageNumberStart97 = 0x501C;
        internal const ushort Orientation = 0x301D;
        internal const ushort RightToLeft = 0x3228;
        internal const ushort PageWidth = 0xB01F;
        internal const ushort PageHeight = 0xB020;
        internal const ushort LeftMargin = 0xB021;
        internal const ushort RightMargin = 0xB022;
        internal const ushort TopMargin = 0x9023;
        internal const ushort BottomMargin = 0x9024;
        internal const ushort Gutter = 0xB025;
        internal const ushort PageNumberStart = 0x7044;

        /// <summary><c>sprmSBrcTop80</c>: the section's top page border, as a four-byte <c>BRC80</c>.</summary>
        /// <remarks>
        /// The four are consecutive — <c>0x702B</c> to <c>0x702E</c> — in top, left, bottom, right
        /// order, which is what lets the reader index a side by subtracting this one.
        /// </remarks>
        internal const ushort BorderTop80 = 0x702B;
        internal const ushort BorderLeft80 = 0x702C;
        internal const ushort BorderBottom80 = 0x702D;
        internal const ushort BorderRight80 = 0x702E;

        /// <summary><c>sprmSPgbProp</c>: which pages the border applies to and what it is measured from.</summary>
        internal const ushort PageBorderProperties = 0x522F;

        /// <summary><c>sprmSBrcTop</c>: the same four in their WW9 form, with an RGB colour.</summary>
        internal const ushort BorderTop = 0xD234;
        internal const ushort BorderLeft = 0xD235;
        internal const ushort BorderBottom = 0xD236;
        internal const ushort BorderRight = 0xD237;
    }
}
