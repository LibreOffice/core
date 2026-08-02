using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.MsBinary.Records;
using Paperless.Presentations.MsBinary;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks that a bullet stated in a symbol face is read as a glyph slot rather than a letter.
/// </summary>
/// <remarks>
/// <para>
/// PowerPoint stores Wingdings' filled circle as <c>0x6C</c> — the letter <c>l</c> in every other
/// face. LibreOffice moves the low byte into the Private Use Area before doing anything with it
/// (<c>PPTNumberFormatCreator::GetNumberFormat</c>, <c>svdfppt.cxx:3767-3771</c>); without that
/// step a deck whose symbol font is not installed comes out with a lower-case L at the head of
/// every bulleted line. Measured on the slides corpus, <c>policy-pesentation.ppt</c> drew fifty-two
/// of them.
/// </para>
/// <para>
/// The recode is checked against a hand-built <c>FontEntityAtom</c> rather than against a corpus
/// deck because <strong>no committed deck can carry the case</strong>: every <c>.ppt</c> in
/// <c>tests/corpus</c> was written by LibreOffice's own exporter, which emits its bullets as
/// StarSymbol code points that are already in the Private Use Area, so the recode is a no-op on
/// all of them. The second test pins that no-op, which is the half a corpus deck can prove.
/// </para>
/// </remarks>
public class PptSymbolBulletTests
{
    /// <summary>
    /// Builds a <c>FontCollection</c> holding one face, with the charset byte the caller names.
    /// </summary>
    /// <remarks>
    /// Sixty-four bytes of UTF-16 name padded with NULs, then <c>lfCharSet</c>, wrapped in an
    /// <c>Environment</c> container because that is where the reader looks for it.
    /// </remarks>
    private static PptFontTable TableWith(string name, byte charSet)
    {
        byte[] entity = new byte[8 + 68];
        BitConverter.GetBytes((ushort)0x0000).CopyTo(entity, 0);
        BitConverter.GetBytes(PptRecordTypes.FontEntityAtom).CopyTo(entity, 2);
        BitConverter.GetBytes(68).CopyTo(entity, 4);
        System.Text.Encoding.Unicode.GetBytes(name).CopyTo(entity, 8);
        entity[8 + 64] = charSet;

        byte[] collection = new byte[8 + entity.Length];
        BitConverter.GetBytes((ushort)0x000F).CopyTo(collection, 0);
        BitConverter.GetBytes(PptRecordTypes.FontCollection).CopyTo(collection, 2);
        BitConverter.GetBytes(entity.Length).CopyTo(collection, 4);
        entity.CopyTo(collection, 8);

        byte[] environment = new byte[8 + collection.Length];
        BitConverter.GetBytes((ushort)0x000F).CopyTo(environment, 0);
        BitConverter.GetBytes(PptRecordTypes.Environment).CopyTo(environment, 2);
        BitConverter.GetBytes(collection.Length).CopyTo(environment, 4);
        collection.CopyTo(environment, 8);

        DffRecordBuffer stream = new(environment);
        stream.TryReadHeader(0, out DffRecordHeader header).ShouldBeTrue();
        return PptFontTable.Read(stream, header);
    }

    [Fact]
    public void ALetterInASymbolFaceBecomesAPrivateUseAreaCodePoint()
    {
        PptFontTable fonts = TableWith("Wingdings", charSet: 2);

        fonts.IsSymbol(0).ShouldBeTrue();
        PptTextReader.Symbolised('l', fonts, font: 0).ShouldBe('');
    }

    [Fact]
    public void ALetterInAnOrdinaryFaceIsLeftAlone()
    {
        PptFontTable fonts = TableWith("Arial", charSet: 0);

        fonts.IsSymbol(0).ShouldBeFalse();
        PptTextReader.Symbolised('l', fonts, font: 0).ShouldBe('l');
    }

    [Fact]
    public void AnAlreadyPrivateUseAreaBulletIsUnchangedByTheRecode()
    {
        // 0xF0B7 & 0xFF is 0xB7, and 0xF000 | 0xB7 is 0xF0B7 again — the case every
        // LibreOffice-exported deck in the corpus takes.
        PptTextReader.Symbolised('', TableWith("StarSymbol", charSet: 2), font: 0)
            .ShouldBe('');
    }

    [Fact]
    public void TheCorpusDecksBulletsStayNormalisedToU2022()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("ppt-features.ppt")));

        List<string> markers = [.. Paragraphs(document.Content)
            .Select(paragraph => paragraph.ListMarker)
            .Where(marker => marker is not null)
            .Select(marker => marker!)];

        markers.ShouldNotBeEmpty();
        markers.ShouldAllBe(marker => marker == "•");
    }

    private static IEnumerable<ContentParagraph> Paragraphs(ContentNode node)
    {
        if (node is ContentParagraph paragraph) yield return paragraph;

        foreach (ContentNode child in node.Children)
        {
            foreach (ContentParagraph found in Paragraphs(child)) yield return found;
        }
    }
}
