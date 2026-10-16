using System.Buffers.Binary;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a DOC's continuous section break does to the page, which is much less than it says.
/// </summary>
/// <remarks>
/// <para>
/// A section descriptor in a DOC restates the entire page setup whatever kind of break it opens with, so
/// a continuous section carries margins Word never applies. LibreOffice settles it in
/// <c>wwSectionManager::InsertSegments</c> (<c>sw/source/filter/ww8/ww8par.cxx</c>:4422): a continuous
/// section that agrees with its predecessor about the sheet becomes a Writer <em>text</em> section, which
/// carries columns and a left/right indent and no page descriptor at all — and an incompatible one gets a
/// page descriptor, which in Writer starts a page.
/// </para>
/// <para>
/// Measured on <c>words/batch-001/doc/foca_form_1.doc</c>, whose second section is continuous, begins
/// inside the opening table, and states a top margin of 1135 twips against the first section's 567.
/// Taking it at its word started the body an inch down every page and pushed the paragraph after the
/// table onto a sheet of its own: four pages against the reference's three.
/// </para>
/// <para>
/// Built against <see cref="Ww8SectionTable.Read"/> rather than against a document. LibreOffice's own DOC
/// export writes one section per page setup and never produces the shape this is about — a continuous
/// break whose restated geometry differs — so no fixture can be generated for it, and the corpus file
/// that has it is 70 kB of form.
/// </para>
/// </remarks>
public sealed class DocSectionGeometryTests
{
    /// <summary>What the first section states, and what both should end up with.</summary>
    private static readonly Length FirstTop = Length.FromTwips(567);

    /// <summary>What the second section restates, and which Word never applies.</summary>
    private static readonly Length SecondTop = Length.FromTwips(1135);

    /// <summary>
    /// The table reader keeps what the descriptor says; dropping it is pagination's job.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This pass used to overwrite the second section's vertical margins with the first's, and that
    /// was wrong rather than merely redundant: a continuous section that states furniture of its own
    /// <em>does</em> get a page descriptor — hung on the first hard page break inside it — and that
    /// descriptor carries the section's own margins. <see cref="ContinuousPageDescriptors"/> already
    /// declines to inherit in exactly that case, and it could not put back what this had erased.
    /// </para>
    /// <para>
    /// The two arms are asserted together below, through the pass that actually decides.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheTableReaderKeepsTheMarginsTheDescriptorStates()
    {
        List<WritingSection> sections = Read(secondIsWider: false);

        sections.Count.ShouldBe(2);
        sections[0].Page.Margins.Top.ShouldBe(FirstTop);
        sections[1].Page.Margins.Top.ShouldBe(
            SecondTop, "what the file says; whether it applies is not this pass's question");
    }

    /// <summary>
    /// A continuous section that names no furniture of its own cannot re-cut the sheet it lands on.
    /// </summary>
    /// <remarks>
    /// The `foca_form_1.doc` case, now asserted where it is decided. Its second section is continuous,
    /// begins inside the opening table and states 1135 twips against the first's 567; taking it at its
    /// word started the body an inch down every page and made four pages of three.
    /// </remarks>
    [Fact]
    public void AContinuousSectionWithoutItsOwnFurnitureLosesItsMargins()
    {
        Paginated(statesOwnFurniture: false, hardBreakInSecond: false)[1]
            .Section.Page.Margins.Top.ShouldBe(FirstTop);
    }

    /// <summary>
    /// A continuous section that names its own furniture and holds a hard page break keeps them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>wwSectionManager::InsertSegments</c>'s <em>"nightmare scenario"</em> arm
    /// (<c>sw/source/filter/ww8/ww8par.cxx</c>:4515-4560): the continuous section has headers and
    /// footers of its own, so a descriptor is built and hung on the first hard page break inside the
    /// section — and a descriptor carries the paper, the margins and the running head together.
    /// </para>
    /// <para>
    /// Measured on <c>words/done-014/doc/PK_FlugzeugeStricken.doc</c>, whose second section is
    /// continuous, names its own running head and states <c>sprmSDyaTop</c> = 1701 twips against the
    /// first's 1417. Patching that 1701 to 2500 moves the reference's body from 85.08 pt to 125.03 and
    /// repaginates it from 7 pages to 11; before this, it moved nothing here at all, and pages 3 to 7
    /// sat 14.20 pt too high — two extra lines by page 6. After: those five pages' unsigned ink goes
    /// from 0.10, 0.08, 0.21, 0.36, 0.54 to 0.01, 0.01, 0.04, 0.05, 0.03.
    /// </para>
    /// </remarks>
    [Fact]
    public void AContinuousSectionWithItsOwnFurnitureAndAHardBreakKeepsThem()
    {
        Paginated(statesOwnFurniture: true, hardBreakInSecond: true)[1]
            .Section.Page.Margins.Top.ShouldBe(SecondTop);
    }

    /// <summary>
    /// Own furniture is not enough on its own: with nowhere to hang the descriptor it is dropped.
    /// </summary>
    /// <remarks>
    /// The control that separates the two conditions, and it is the branch `wwSectionManager` writes
    /// as <c>if (bFailed) aIter-&gt;mpPage = pOrig</c> — the search for a hard break found none, so the
    /// section wears the descriptor above it after all.
    /// </remarks>
    [Fact]
    public void OwnFurnitureWithNoHardBreakStillLosesTheMargins()
    {
        Paginated(statesOwnFurniture: true, hardBreakInSecond: false)[1]
            .Section.Page.Margins.Top.ShouldBe(FirstTop);
    }

    /// <summary>The two sections as pagination settles them, over two one-paragraph sections.</summary>
    private static IReadOnlyList<PaginatedSection> Paginated(
        bool statesOwnFurniture, bool hardBreakInSecond)
    {
        List<WritingSection> read = Read(secondIsWider: false);

        List<PaginatedSection> sections =
        [
            new PaginatedSection(read[0], SideMarginsAreASectionIndent: true),
            new PaginatedSection(
                read[1], StatesOwnFurniture: statesOwnFurniture, SideMarginsAreASectionIndent: true),
        ];

        List<PageBlock> blocks =
        [
            Paragraph(0, startsNewPage: false),
            Paragraph(1, startsNewPage: hardBreakInSecond),
        ];

        return ContinuousPageDescriptors.Resolve(sections, blocks);
    }

    private static PageParagraph Paragraph(int section, bool startsNewPage) => new()
    {
        Text = "x",
        Face = Face,
        SectionIndex = section,
        Format = ParagraphFormat.Default with { StartsNewPage = startsNewPage },
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }

    [Fact]
    public void ACompatibleContinuousSectionStillTakesItsOwnSideMargins()
    {
        List<WritingSection> sections = Read(secondIsWider: false);

        // InsertSection puts the difference between the two on the text section as an indent
        // (ww8par6.cxx:762), so the side margins really do change where a continuous break does not.
        sections[1].Page.Margins.Left.ShouldBe(Length.FromTwips(2000));
        sections[1].Break.ShouldBe(SectionBreak.Continuous);
    }

    [Fact]
    public void AContinuousSectionOnDifferentPaperBecomesAPageBreak()
    {
        List<WritingSection> sections = Read(secondIsWider: true);

        // "If two following sections are different in following properties, Word will interpret a
        // continuous section break between them as if it was a section break next page" — ww8par.cxx:4419.
        sections[1].Break.ShouldBe(SectionBreak.NextPage);
        sections[1].Page.Margins.Top.ShouldBe(SecondTop, "a fresh sheet is cut to its own measure");
    }

    /// <summary>
    /// A two-section <c>PlcfSed</c> over a <c>WordDocument</c> holding both property sets.
    /// </summary>
    /// <param name="secondIsWider">
    /// True to give the second section a different paper width, which is what decides whether its
    /// continuous break can be honoured.
    /// </param>
    private static List<WritingSection> Read(bool secondIsWider)
    {
        byte[] first = Sepx(top: 567, left: 1134, width: 16840, continuous: false);
        byte[] second = Sepx(
            top: 1135, left: 2000, width: secondIsWider ? 11907 : 16840, continuous: true);

        // The two grpprls land in the WordDocument stream, which is where an SED's offset points —
        // the one indirection in this table that is easy to read as an offset into the table stream.
        byte[] wordDocument = new byte[first.Length + second.Length];
        first.CopyTo(wordDocument, 0);
        second.CopyTo(wordDocument, first.Length);

        Ww8Plcf descriptors = Ww8Plcf.Parse(
            PlcfSed(0, 189, 11421, 0, first.Length), Ww8SectionTable.DescriptorSize);

        return Ww8SectionTable.Read(descriptors, wordDocument);
    }

    /// <summary>A length-prefixed section grpprl, as the <c>WordDocument</c> stream stores one.</summary>
    private static byte[] Sepx(int top, int left, int width, bool continuous)
    {
        List<byte> sprms = [];

        if (continuous) Add(sprms, 0x3009, [0]);      // sprmSBkc, zero being continuous
        Add(sprms, 0xB01F, Word(width));              // sprmSXaPage
        Add(sprms, 0xB020, Word(11907));              // sprmSYaPage
        Add(sprms, 0xB021, Word(left));               // sprmSDxaLeft
        Add(sprms, 0xB022, Word(1134));               // sprmSDxaRight
        Add(sprms, 0x9023, Word(top));                // sprmSDyaTop
        Add(sprms, 0x9024, Word(357));                // sprmSDyaBottom

        byte[] result = new byte[2 + sprms.Count];
        BinaryPrimitives.WriteUInt16LittleEndian(result, (ushort)sprms.Count);
        sprms.CopyTo(result, 2);
        return result;

        static void Add(List<byte> into, ushort identifier, byte[] operand)
        {
            into.Add((byte)(identifier & 0xFF));
            into.Add((byte)(identifier >> 8));
            into.AddRange(operand);
        }

        static byte[] Word(int value)
        {
            byte[] bytes = new byte[2];
            BinaryPrimitives.WriteInt16LittleEndian(bytes, (short)value);
            return bytes;
        }
    }

    /// <summary>A <c>PlcfSed</c> of two sections, each pointing at one of the grpprls.</summary>
    private static byte[] PlcfSed(int start, int middle, int end, int firstAt, int secondAt)
    {
        const int Descriptor = Ww8SectionTable.DescriptorSize;

        byte[] bytes = new byte[(3 * 4) + (2 * Descriptor)];
        BinaryPrimitives.WriteInt32LittleEndian(bytes, start);
        BinaryPrimitives.WriteInt32LittleEndian(bytes.AsSpan(4), middle);
        BinaryPrimitives.WriteInt32LittleEndian(bytes.AsSpan(8), end);

        // An SED's second field is the offset of its properties; the rest is bookkeeping this ignores.
        BinaryPrimitives.WriteInt32LittleEndian(bytes.AsSpan(12 + 2), firstAt);
        BinaryPrimitives.WriteInt32LittleEndian(bytes.AsSpan(12 + Descriptor + 2), secondAt);
        return bytes;
    }
}
