using System.Buffers.Binary;
using System.Text;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>.doc</c>'s table of contents keeps the file's own <c>Hyperlink</c> character style, and the
/// reference throws exactly that one away.
/// </summary>
/// <remarks>
/// <para>
/// Word writes each entry of a hyperlinked contents list as a run naming the built-in
/// <c>Hyperlink</c> character style — blue and single-underlined — and LibreOffice rebuilds the index's
/// links in the empty <c>Index Link</c> pool style instead. Honouring the file's style draws the whole
/// list blue and underlined: on <c>150_5335_5a.doc</c> that is 29 251 pt of underline over three
/// contents pages where 26.2.4.2 draws none at all.
/// </para>
/// <para>
/// <strong>The WW8 rule is not the DOCX one and does not transfer from it.</strong> The OOXML reader
/// drops <em>every</em> character style inside a TOC (<c>DomainMapper.cxx</c>:3037-3047, which
/// <c>WordParagraphFormats.ResolveRun</c>'s <c>ignoreCharacterStyle</c> models); the WW8 reader drops
/// exactly one, in the sprm handler — <c>SwWW8ImplReader::Read_CColl</c>
/// (<c>sw/source/filter/ww8/ww8par6.cxx</c>:4135-4160) returns early when
/// <c>m_bLoadingTOXCache &amp;&amp; m_vColl[nId].GetWWStyleId() == ww::stiHyperlink</c>. So the two
/// halves under test here are <em>which</em> style (its <c>sti</c>, not its name and not any character
/// style) and <em>where</em> (the field's cached result, which spans every entry's paragraph).
/// </para>
/// <para>
/// Unit rather than document tests because the corpus holds the only <c>.doc</c> that exercise it and
/// the repository holds no fixture with a hyperlinked contents list; each arm below was instead
/// checked by mutation, by removing the clause it names and watching it fail.
/// </para>
/// </remarks>
public sealed class Ww8IndexLinkStyleTests
{
    private const char Begin = '';
    private const char Separator = '';
    private const char End = '';

    /// <summary>The <c>TOC</c> field's type byte in the <c>PlcFld</c>.</summary>
    private const byte Toc = 13;

    /// <summary>The <c>PAGEREF</c> field's, which is what a contents entry nests inside itself.</summary>
    private const byte PageRef = 37;

    /// <summary>
    /// A style's built-in identity is read, and it is the low twelve bits of the STD's first word.
    /// </summary>
    /// <remarks>
    /// The four bits above it are <c>fScratch</c>, <c>fInvalHeight</c>, <c>fHasUpe</c> and
    /// <c>fMassCopy</c>; a reader taking the whole word answers 0x8055 for a hyperlink style whose
    /// <c>fMassCopy</c> is set and matches nothing.
    /// </remarks>
    [Fact]
    public void AStylesBuiltInIdentityIsTheLowTwelveBitsOfItsFirstWord()
    {
        Ww8StyleSheet sheet = Sheet(
            Style(Ww8Style.UserStyle, kind: 1, baseIndex: Ww8Style.NoBaseStyle, "Normal"),
            Style(Ww8Style.HyperlinkStyle, kind: 2, baseIndex: 0, "Hyperlink"),
            Style((ushort)(0x8000 | Ww8Style.HyperlinkStyle), kind: 2, baseIndex: 0, "Flagged"));

        sheet.At(0)!.Value.Sti.ShouldBe(Ww8Style.UserStyle);
        sheet.At(1)!.Value.Sti.ShouldBe(Ww8Style.HyperlinkStyle);
        sheet.At(2)!.Value.Sti.ShouldBe(Ww8Style.HyperlinkStyle);
    }

    /// <summary>
    /// The style an index result drops is the built-in <c>Hyperlink</c> one, by number.
    /// </summary>
    /// <remarks>
    /// Every other arm is a style the reference keeps, and each is a different way of being nearly the
    /// right style: <c>FollowedHyperlink</c> is <c>stiHyperlinkFollowed</c> and the test
    /// <c>Read_CColl</c> makes is against <c>stiHyperlink</c> alone; a style of the document's own that
    /// happens to be called <c>Hyperlink</c> is <c>stiUser</c>; and a <em>paragraph</em> style carrying
    /// the number is not a character style at all, which is the guard <c>Read_CColl</c> makes two lines
    /// earlier (<c>|| m_vColl[nId].m_bColl</c>).
    /// </remarks>
    [Fact]
    public void OnlyTheBuiltInHyperlinkCharacterStyleIsTheIndexLinkStyle()
    {
        Ww8StyleSheet sheet = Sheet(
            Style(Ww8Style.UserStyle, kind: 1, baseIndex: Ww8Style.NoBaseStyle, "Normal"),
            Style(Ww8Style.HyperlinkStyle, kind: 2, baseIndex: 0, "Hyperlink"),
            Style(86, kind: 2, baseIndex: 0, "FollowedHyperlink"),
            Style(Ww8Style.UserStyle, kind: 2, baseIndex: 0, "Hyperlink"),
            Style(Ww8Style.HyperlinkStyle, kind: 1, baseIndex: 0, "Hyperlink"));

        Ww8DocumentReader.IsIndexLinkStyle(sheet, 1).ShouldBeTrue();

        Ww8DocumentReader.IsIndexLinkStyle(sheet, 2).ShouldBeFalse("stiHyperlinkFollowed is 86");
        Ww8DocumentReader.IsIndexLinkStyle(sheet, 3).ShouldBeFalse("a style of the document's own");
        Ww8DocumentReader.IsIndexLinkStyle(sheet, 4).ShouldBeFalse("a paragraph style");
        Ww8DocumentReader.IsIndexLinkStyle(sheet, 0).ShouldBeFalse("Normal");
        Ww8DocumentReader.IsIndexLinkStyle(sheet, 99).ShouldBeFalse("no such style");
    }

    /// <summary>
    /// An index field's result runs from its separator to its end, across every paragraph between.
    /// </summary>
    /// <remarks>
    /// The extent is the whole point. A contents list is one field: its <c>U+0013</c>, its instruction
    /// and its <c>U+0014</c> sit in the first entry's paragraph and its <c>U+0015</c> in the last, so a
    /// state reset at each paragraph mark suppresses the style on entry one and on nothing else — which
    /// is the defect round 126 found on the DOCX side and closed with a document-level counter.
    /// </remarks>
    [Fact]
    public void AnIndexResultSpansEveryEntrysParagraph()
    {
        string text = Begin + " TOC " + Separator + "One\rTwo\rThree" + End + "after";
        List<Ww8Range> ranges = Ww8DocumentReader.IndexResultRanges(text, 0, Fields((0, Toc)), 0);

        Ww8Range range = ranges.ShouldHaveSingleItem();
        range.Start.ShouldBe(text.IndexOf(Separator) + 1);
        range.End.ShouldBe(text.IndexOf(End));
        text[range.Start..range.End].ShouldBe("One\rTwo\rThree");
    }

    /// <summary>
    /// The <c>PAGEREF</c> fields inside each entry do not end it.
    /// </summary>
    /// <remarks>
    /// A hyperlinked entry is a <c>HYPERLINK</c> or <c>PAGEREF</c> field nested in the index's result —
    /// <c>150_5335_5a.doc</c> holds 122 of each — so a rule that closed on the first <c>U+0015</c> would
    /// cover the first entry and stop. <c>End_Field</c> distinguishes them by the field's <em>type</em>
    /// (<c>ww8par5.cxx</c>:595-626) and so does this.
    /// </remarks>
    [Fact]
    public void ANestedFieldInsideAnEntryDoesNotEndTheResult()
    {
        string text = Begin + " TOC " + Separator + "One" + Begin + " PAGEREF " + Separator + "3" + End
                      + "\rTwo" + End + "after";
        List<Ww8Range> ranges = Ww8DocumentReader.IndexResultRanges(
            text, 0, Fields((0, Toc), (text.IndexOf(Begin, 1), PageRef)), 0);

        Ww8Range range = ranges.ShouldHaveSingleItem();
        range.End.ShouldBe(text.LastIndexOf(End));
    }

    /// <summary>
    /// A field that is not an index contributes no range, and neither does an index's instruction.
    /// </summary>
    /// <remarks>
    /// The negative arm of both halves of the rule. A <c>PAGEREF</c> standing on its own is the field
    /// type this walk must ignore, and the instruction of an index field is the half of it that is not
    /// the result — nothing there is drawn, and treating the whole field as the result would put the
    /// suppression a few characters early on every document that has one.
    /// </remarks>
    [Fact]
    public void AFieldThatIsNotAnIndexContributesNothing()
    {
        string plain = Begin + " PAGEREF " + Separator + "3" + End;
        Ww8DocumentReader.IndexResultRanges(plain, 0, Fields((0, PageRef)), 0).ShouldBeEmpty();

        string noSeparator = Begin + " TOC " + End;
        Ww8DocumentReader.IndexResultRanges(noSeparator, 0, Fields((0, Toc)), 0).ShouldBeEmpty();

        Ww8DocumentReader.IndexResultRanges("plain text", 0, Ww8FieldTypes.Empty, 0).ShouldBeEmpty();
    }

    /// <summary>
    /// Positions are the walk's own, so a story that does not start at nought still pairs correctly.
    /// </summary>
    /// <remarks>
    /// A footnote's or a header's field table counts from its own story's start while the walk's
    /// positions are the whole document's, which is what <c>FieldTypesOf</c>'s base exists to
    /// reconcile — look a global position up in a story-relative table and the type comes back null,
    /// so every field in that story reads as type 0 and no index is ever found.
    /// </remarks>
    [Fact]
    public void AStoryThatDoesNotStartAtNoughtPairsAgainstItsOwnBase()
    {
        const int start = 5000;
        const int fieldBase = 4000;
        string text = Begin + " TOC " + Separator + "One" + End;

        List<Ww8Range> ranges = Ww8DocumentReader.IndexResultRanges(
            text, start, Fields((start - fieldBase, Toc)), fieldBase);

        Ww8Range range = ranges.ShouldHaveSingleItem();
        range.Start.ShouldBe(start + text.IndexOf(Separator) + 1);
        range.End.ShouldBe(start + text.IndexOf(End));
    }

    /// <summary>
    /// An index field the story ends inside still covers what it drew.
    /// </summary>
    /// <remarks>
    /// Leniency rather than a rule of the format: a document whose markers and field table disagree is
    /// one Word itself renders, and the walk that reads the text keeps its own stack for the same
    /// reason. Dropping the range instead would draw the contents list of a truncated document blue.
    /// </remarks>
    [Fact]
    public void AnIndexFieldLeftOpenReachesTheEndOfTheStory()
    {
        string text = Begin + " TOC " + Separator + "One\rTwo";
        List<Ww8Range> ranges = Ww8DocumentReader.IndexResultRanges(text, 0, Fields((0, Toc)), 0);

        Ww8Range range = ranges.ShouldHaveSingleItem();
        range.End.ShouldBe(text.Length);
    }

    /// <summary>A <c>PlcFld</c> naming one type per field beginning.</summary>
    /// <remarks>
    /// Two four-byte positions and one two-byte record per field half, which is the shape
    /// <see cref="Ww8FieldTypes.Parse"/> reads: <c>FLD.ch</c> in the low five bits of the first byte,
    /// and for a beginning the type in the second.
    /// </remarks>
    private static Ww8FieldTypes Fields(params (int Position, byte Type)[] beginnings)
    {
        List<byte> positions = [];
        List<byte> records = [];

        foreach ((int position, byte type) in beginnings)
        {
            byte[] four = new byte[4];
            BinaryPrimitives.WriteInt32LittleEndian(four, position);
            positions.AddRange(four);
            records.AddRange([19, type]);
        }

        // One position more than there are records, as a PLCF always has.
        positions.AddRange([0xFF, 0xFF, 0x7F, 0x7F]);
        return Ww8FieldTypes.Parse([.. positions, .. records]);
    }

    /// <summary>One STD, with the built-in identity this file is about in its first word.</summary>
    private static byte[] Style(ushort sti, byte kind, ushort baseIndex, string name)
    {
        ushort kindAndBase = (ushort)((baseIndex << 4) | kind);
        List<byte> std = [(byte)(sti & 0xFF), (byte)(sti >> 8), (byte)(kindAndBase & 0xFF), (byte)(kindAndBase >> 8)];

        std.AddRange(new byte[18 - std.Count]);
        std.Add((byte)name.Length);
        std.Add(0);
        std.AddRange(Encoding.Unicode.GetBytes(name));
        std.AddRange([0, 0]);

        // A paragraph style's first UPX is its PAPX, which opens with its own istd.
        if (kind == 1) std.AddRange([2, 0, 0, 0]);

        std.AddRange([0, 0]);
        return [.. std];
    }

    /// <summary>An STSH holding the styles, with the eighteen-byte fixed part these STDs use.</summary>
    private static Ww8StyleSheet Sheet(params byte[][] styles)
    {
        byte[] header = new byte[18];
        BinaryPrimitives.WriteUInt16LittleEndian(header, (ushort)styles.Length);
        BinaryPrimitives.WriteUInt16LittleEndian(header.AsSpan(2), 18);

        List<byte> bytes = [(byte)header.Length, (byte)(header.Length >> 8), .. header];
        foreach (byte[] style in styles)
        {
            bytes.Add((byte)style.Length);
            bytes.Add((byte)(style.Length >> 8));
            bytes.AddRange(style);
            if ((style.Length & 1) != 0) bytes.Add(0);
        }

        return Ww8StyleSheet.Parse([.. bytes]);
    }
}
