using System.Buffers.Binary;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Ww8;

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
    /// break is promoted rather than honoured.
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
                continue;
            }

            sections[i] = section with
            {
                Page = section.Page with
                {
                    Size = carried.Size,
                    IsLandscape = carried.IsLandscape,
                    HasMirroredMargins = carried.HasMirroredMargins,
                    HeaderDistance = carried.HeaderDistance,
                    FooterDistance = carried.FooterDistance,
                    HeaderHeight = carried.HeaderHeight,
                    FooterHeight = carried.FooterHeight,

                    // The left and right margins are the exception, and they are an exception in
                    // LibreOffice too: a text section takes them as an indent relative to the page's own,
                    // which narrows the text area by exactly the difference.
                    Margins = section.Page.Margins with
                    {
                        Top = carried.Margins.Top,
                        Bottom = carried.Margins.Bottom,
                    },
                },
            };
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

    private static WritingSection ReadProperties(ReadOnlyMemory<byte> grpprl)
    {
        PageGeometry page = PageGeometry.Default;
        PageMargins margins = PageMargins.Default;
        DocSize size = page.Size;

        SectionBreak sectionBreak = SectionBreak.NextPage;
        Length gutter = Length.Zero;
        Length headerDistance = Length.Zero;
        Length footerDistance = Length.Zero;
        Length columnGap = Length.Zero;
        int columns = 1;
        bool landscape = false;
        bool rightToLeft = false;
        bool titlePage = false;
        int? restartAt = null;
        bool restartsNumbering = false;

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

                case Sprms.RestartsPageNumbering:
                    restartsNumbering = sprm.Byte != 0;
                    break;
                case Sprms.PageNumberStart97:
                    restartAt = sprm.Word;
                    break;
                case Sprms.PageNumberStart:
                    restartAt = sprm.DoubleWord;
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
                IsLandscape = landscape,
                IsRightToLeft = rightToLeft,
            },

            Break = sectionBreak,

            // The start value only applies when the section says it restarts. A document that carries
            // a stale start value from an earlier edit is common, and honouring it renumbers pages
            // that Word numbers continuously.
            RestartPageNumberAt = restartsNumbering ? restartAt ?? 1 : null,
            HasDifferentFirstPage = titlePage,
        };
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
    private static Length? Dimension(Ww8Sprm sprm)
    {
        int twips = sprm.Word;
        return twips is > 0 and <= 22 * 1440 ? Length.FromTwips(twips) : null;
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
        internal const ushort RestartsPageNumbering = 0x3011;
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
    }
}
