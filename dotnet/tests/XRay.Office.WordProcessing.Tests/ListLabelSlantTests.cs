using XRay.Office.Text.Fonts;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Whether a list label leans, and which of the three places that state <c>w:i</c> decides it.
/// </summary>
/// <remarks>
/// <para>
/// Measured on 26.2.4.2 over sixteen authored packages in four formats —
/// <c>dotnet/probes/words-r59/label-slant.py</c> — and the answer is two rules rather than one,
/// because <c>SwTextFormatter::NewNumberPortion</c> builds a bullet's font and a number's font
/// differently (<c>sw/source/core/text/txtfld.cxx</c>:578-600).
/// </para>
/// <list type="bullet">
/// <item>
/// <strong>The level wins outright when it states anything, in either direction.</strong> A level
/// stating <c>&lt;w:i w:val="0"/&gt;</c> draws an upright label over an italic paragraph and a level
/// stating <c>&lt;w:i/&gt;</c> draws a leaning one over an upright paragraph. Thirteen of the words
/// corpus's 271 <c>.docx</c> write the first of those shapes.
/// </item>
/// <item>
/// <strong>A bullet's base font has had its posture reset and a number's has not</strong>
/// (<c>#i53199</c>). So a paragraph <em>style</em>'s <c>w:i</c> leans the number and leaves the
/// bullet upright, while the same <c>w:i</c> written directly on the paragraph's own
/// <c>w:pPr/w:rPr</c> leans both — the second reaches the bullet through
/// <c>checkApplyParagraphMarkFormatToNumbering</c>, which the style chain does not.
/// </item>
/// </list>
/// <para>
/// The bullet is asserted through <see cref="Core.Graphics.FontReference.SyntheticOblique"/> and the
/// number through the resolved face, and that difference is the subject rather than an accident:
/// OpenSymbol ships one cut, so its lean can only be synthetic, and Liberation Serif ships an italic,
/// so its lean is a different font file. Reading only one of the two would miss half the rule.
/// </para>
/// <para>
/// Reintroducing the bug to check these fail: have <c>DocxLayoutSource.LabelFace</c> pass
/// <c>italic: false</c> to <c>Symbol</c>, or drop the <c>levelItalic ??</c> from either branch.
/// </para>
/// </remarks>
public sealed class ListLabelSlantTests
{
    /// <summary>A bullet leans when its level says so, and not when its level says not to.</summary>
    [Theory]
    [InlineData("bullet control", false)]
    [InlineData("bullet level", true)]
    [InlineData("bullet mark", true)]
    [InlineData("bullet style", false)]
    [InlineData("bullet leveloff markon", false)]
    [InlineData("bullet levelon markoff", true)]
    public void ABulletTakesTheLevelsPostureAndThenTheParagraphMarksOwn(string item, bool leans)
    {
        PageLabel label = Label(item);

        SymbolFontRecode.IsSubstituteFamily(label.Font?.FamilyName).ShouldBeTrue(
            $"'{item}' was not recoded into the substitute face at all");

        label.Font!.SyntheticOblique.ShouldBe(leans);

        // The lean is synthetic and not a second file: asserting the flag alone would pass a
        // resolution that had found some other family's italic and drawn the wrong picture.
        label.Face.IsItalic.ShouldBeFalse();
    }

    /// <summary>
    /// A number takes the paragraph's posture, and the level overrides it in either direction.
    /// </summary>
    [Theory]
    [InlineData("number control", false)]
    [InlineData("number level", true)]
    [InlineData("number mark", true)]
    [InlineData("number style", true)]
    [InlineData("number leveloff markon", false)]
    [InlineData("number levelon markoff", true)]
    public void ANumberKeepsTheParagraphsPostureUnlessTheLevelStatesOne(string item, bool leans)
    {
        PageLabel label = Label(item);

        // Liberation Serif has an italic installed, so the request is answered with a face and not
        // with a shear — which is the half of the rule the bullet cases cannot see.
        label.Face.IsItalic.ShouldBe(leans);
        label.Font!.SyntheticOblique.ShouldBeFalse();
    }

    /// <summary>
    /// The item's own text is unaffected by any of it, in both directions.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The control that stops the rule being satisfied by leaning the whole paragraph. "number
    /// levelon markoff" and "bullet mark" each lean their <em>label</em> while their text stays
    /// upright, and a change that moved the paragraph's posture instead of the label's would pass
    /// every assertion above and fail these.
    /// </para>
    /// <para>
    /// <strong>A paragraph mark's <c>w:i</c> does not lean the item's text</strong>, which is why
    /// "bullet mark" expects false here and true above: <c>w:pPr/w:rPr</c> is the formatting of the
    /// mark character, and a run carrying its own <c>w:rPr</c> is unaffected by it. The reference
    /// agrees — its <c>02-bullet-mark.docx</c> draws eight upright <c>LiberationSerif</c> glyphs
    /// beside one sheared OpenSymbol one. A paragraph <em>style</em>'s <c>w:i</c> is the opposite
    /// case and leans the text and not the bullet, which is the last row.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("number levelon markoff", false)]
    [InlineData("number leveloff markon", false)]
    [InlineData("bullet mark", false)]
    [InlineData("bullet style", true)]
    public void TheItemsOwnTextIsUnmoved(string item, bool leans)
    {
        Paragraph(item).Face.IsItalic.ShouldBe(leans);
    }

    private static PageLabel Label(string item)
    {
        PageLabel? label = Paragraph(item).Label;
        label.ShouldNotBeNull($"'{item}' drew no label");
        return label!;
    }

    private static PageParagraph Paragraph(string item)
    {
        using MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "slant.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Paragraphs.Single(paragraph => paragraph.Text == item);
    }

    /// <summary>
    /// One DOCX holding both label kinds and all three places a posture can be stated.
    /// </summary>
    /// <remarks>
    /// Six abstract definitions rather than one, because the level's own <c>w:rPr</c> is one of the
    /// three variables and a level cannot state two things at once. Built here rather than committed
    /// so the whole rule fits on one screen beside the assertions that read it.
    /// </remarks>
    private static MemoryStream BuildPackage()
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
              <Override PartName="/word/numbering.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
              <Relationship Id="rId2" Target="numbering.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"/>
            </Relationships>
            """;

        // U+F0B7 is Symbol's bullet slot, which nothing on a Linux box can draw, so LibreOffice
        // recodes it into OpenSymbol -- the face whose single cut makes the lean synthetic.
        string numbering =
            $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:numbering xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              {Bullet(0, "")}
              {Bullet(1, "<w:i/>")}
              {Bullet(2, "<w:i w:val=\"0\"/>")}
              {Number(3, "")}
              {Number(4, "<w:i/>")}
              {Number(5, "<w:i w:val=\"0\"/>")}
              <w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
              <w:num w:numId="2"><w:abstractNumId w:val="1"/></w:num>
              <w:num w:numId="3"><w:abstractNumId w:val="2"/></w:num>
              <w:num w:numId="4"><w:abstractNumId w:val="3"/></w:num>
              <w:num w:numId="5"><w:abstractNumId w:val="4"/></w:num>
              <w:num w:numId="6"><w:abstractNumId w:val="5"/></w:num>
            </w:numbering>
            """;

        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault><w:rPr>
                  <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                  <w:sz w:val="24"/>
                </w:rPr></w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
              <w:style w:type="paragraph" w:styleId="Italicised">
                <w:name w:val="Italicised"/>
                <w:basedOn w:val="Normal"/>
                <w:rPr><w:i/></w:rPr>
              </w:style>
            </w:styles>
            """;

        string document =
            $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {Item("bullet control", 1, null, "")}
                {Item("bullet level", 2, null, "")}
                {Item("bullet mark", 1, null, "<w:i/>")}
                {Item("bullet style", 1, "Italicised", "")}
                {Item("bullet leveloff markon", 3, null, "<w:i/>")}
                {Item("bullet levelon markoff", 2, null, "<w:i w:val=\"0\"/>")}
                {Item("number control", 4, null, "")}
                {Item("number level", 5, null, "")}
                {Item("number mark", 4, null, "<w:i/>")}
                {Item("number style", 4, "Italicised", "")}
                {Item("number leveloff markon", 6, null, "<w:i/>")}
                {Item("number levelon markoff", 5, null, "<w:i w:val=\"0\"/>")}
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
            Write(archive, "word/numbering.xml", numbering);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static string Bullet(int identifier, string posture)
            => $"""
               <w:abstractNum w:abstractNumId="{identifier}"><w:lvl w:ilvl="0">
                 <w:start w:val="1"/><w:numFmt w:val="bullet"/>
                 <w:lvlText w:val=""/>
                 <w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>
                 <w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol"/>{posture}</w:rPr>
               </w:lvl></w:abstractNum>
               """;

        static string Number(int identifier, string posture)
            => $"""
               <w:abstractNum w:abstractNumId="{identifier}"><w:lvl w:ilvl="0">
                 <w:start w:val="1"/><w:numFmt w:val="decimal"/><w:lvlText w:val="%1."/>
                 <w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>
                 <w:rPr>{posture}</w:rPr>
               </w:lvl></w:abstractNum>
               """;

        static string Item(string text, int instance, string? style, string mark)
            => $"""
               <w:p><w:pPr>
                 {(style is null ? string.Empty : $"<w:pStyle w:val=\"{style}\"/>")}
                 <w:numPr><w:ilvl w:val="0"/><w:numId w:val="{instance}"/></w:numPr>
                 <w:rPr>{mark}</w:rPr>
               </w:pPr><w:r><w:t>{text}</w:t></w:r></w:p>
               """;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
