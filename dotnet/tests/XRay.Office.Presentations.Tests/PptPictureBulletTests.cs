using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Records;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A binary PowerPoint picture bullet: where the deck states it, how big it is drawn, and what
/// the layout does with a label that is a graphic rather than a character.
/// </summary>
/// <remarks>
/// <para>
/// Every expected number here is 26.2.4.2's own, and two of them are its own output rather than
/// its source: the size rule is checked against the <c>text:list-level-style-image</c> elements in
/// its flat ODP of
/// <c>slides/done-003/ppt/ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt</c>, and
/// the placement rule against where it puts that deck's page-8 bullets in its own PDF —
/// six 17.86 × 17.86 pt graphics whose top-left corners are (43.1, 128.4), (43.1, 164.7),
/// (43.1, 317.5), (43.1, 348.7), (43.1, 379.8) and (43.1, 432.5).
/// </para>
/// <para>
/// The record layouts come from the <c>27.2.0.0.alpha0+</c> checkout and are the explanation;
/// what pins them is that reading them this way reproduces the eight stated sizes exactly.
/// </para>
/// </remarks>
public class PptPictureBulletTests
{
    /// <summary>A one-pixel PNG, small enough to inline and real enough to sniff.</summary>
    private static readonly byte[] Png =
    [
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xDE,
    ];

    // ------------------------------------------------------------------ the master's own levels

    [Fact]
    public void AMasterStatesAPictureBulletPerOutlineLevelAndTextKind()
    {
        // The shape of every one of the six atoms in the corpus deck: depth five, levels 0 and 1
        // carrying `mask=0x03800000 buBlip=n hasAnm=0 anmScheme=0x00010003`, levels 2 to 4 empty.
        byte[] stream = MainMaster(
            (PptTextKind.Body, [Blip(0), Blip(1), Unset(), Unset(), Unset()]),
            (PptTextKind.HalfBody, [Blip(2), Blip(3), Unset(), Unset(), Unset()]));

        PptExtendedParagraphSheet sheet = Read(stream);

        sheet.IsEmpty.ShouldBeFalse();
        sheet.Level(PptTextKind.Body, 0).BulletBlip.ShouldBe((ushort)0);
        sheet.Level(PptTextKind.Body, 1).BulletBlip.ShouldBe((ushort)1);
        sheet.Level(PptTextKind.HalfBody, 0).BulletBlip.ShouldBe((ushort)2);
        sheet.Level(PptTextKind.HalfBody, 1).BulletBlip.ShouldBe((ushort)3);

        // A level the atom did not write states nothing, and a text kind it did not mention
        // states nothing either — neither may inherit the level beside it.
        sheet.Level(PptTextKind.Body, 2).IsSet.ShouldBeFalse();
        sheet.Level(PptTextKind.Title, 0).IsSet.ShouldBeFalse();
    }

    [Fact]
    public void ALevelEndsAtItsCharacterMaskWhereAShapesEntryHasAThirdMaskAfterIt()
    {
        // `ReadPPTExtParaLevel` (svdfppt.cxx:3185-3201) and `StyleTextProp9::Read` (:4812-4830)
        // agree field for field as far as the character mask, and then the paragraph's entry
        // reads a special-info mask with two optional fields that a level does not have. Every
        // field is optional, so reading a level with the paragraph's reader consumes four bytes
        // too many and takes the NEXT level from the wrong offset rather than losing one value.
        // Level 1's blip is the assertion that carries that: it is only 7 if level 0 consumed
        // exactly its own sixteen bytes.
        byte[] level0 = [.. Mask(0x03800000), .. U16(9), .. U16(1), .. U32(0x0001_0008), .. U32(0)];
        byte[] level1 = [.. Mask(0x00800000), .. U16(7), .. U32(0)];

        byte[] stream = MainMaster((PptTextKind.Body, [level0, level1]));
        PptExtendedParagraphSheet sheet = Read(stream);

        sheet.Level(PptTextKind.Body, 0).BulletBlip.ShouldBe((ushort)9);
        sheet.Level(PptTextKind.Body, 0).HasAutoNumber.ShouldBeTrue();
        sheet.Level(PptTextKind.Body, 0).Scheme.ShouldBe(0x0001_0008u);
        sheet.Level(PptTextKind.Body, 1).BulletBlip.ShouldBe((ushort)7);
    }

    // ------------------------------------------------------------------------------ the merge

    [Fact]
    public void AParagraphStatingNothingTakesTheMastersPictureBullet()
    {
        // The case the corpus deck is: `mnExtParagraphMask` zero on every paragraph, the blip on
        // the master's level. This is what round 110 found inert when it looked at the
        // paragraph's own entry alone.
        PptExtendedParagraph merged = PptTextBody.MergedExtension(
            null, new PptExtendedParagraphLevel(0x03800000, BulletBlip: 4));

        merged.BulletBlip.ShouldBe((ushort)4);
    }

    [Fact]
    public void AParagraphStatingAllThreeFieldsIsNotMergedAtAll()
    {
        // `if ((nBuFlags & 0x03800000) != 0x03800000)` guards the whole merge (svdfppt.cxx:3421).
        PptExtendedParagraph own = new(0x03800000, PptExtendedParagraph.NoBulletBlip, false, 0);

        PptTextBody.MergedExtension(own, new PptExtendedParagraphLevel(0x03800000, BulletBlip: 4))
            .BulletBlip.ShouldBe(PptExtendedParagraph.NoBulletBlip);
    }

    [Fact]
    public void AParagraphStatingOnlyAutomaticNumberingSuppressesTheLevelsPicture()
    {
        // "if there is a BuStart without BuInstance, then there is no graphical Bullet possible"
        // — the extra `if (!(nBuFlags & 0x02000000))` inside the blip arm (svdfppt.cxx:3436-3440).
        // The scheme still merges, which is what makes this a condition on the blip alone.
        PptExtendedParagraph own = new(0x02000000, PptExtendedParagraph.NoBulletBlip, true, 0);
        PptExtendedParagraphLevel level = new(0x03800000, BulletBlip: 4, Scheme: 0x0001_0003);

        PptExtendedParagraph merged = PptTextBody.MergedExtension(own, level);

        merged.BulletBlip.ShouldBe(PptExtendedParagraph.NoBulletBlip);
        merged.HasAutoNumber.ShouldBeTrue();
        merged.Scheme.ShouldBe(0x0001_0003u);
    }

    [Fact]
    public void AMasterThatWroteNoLevelMergesNothing()
    {
        // `PPTExtParaProv::bStyles` (svdfppt.cxx:3422). An unwritten level is all zeroes, and
        // merging those would clear a paragraph's own scheme rather than leave it alone.
        PptExtendedParagraph own = new(0x01000000, PptExtendedParagraph.NoBulletBlip, false, 0x0001_0007);

        PptTextBody.MergedExtension(own, default).Scheme.ShouldBe(0x0001_0007u);
    }

    // ------------------------------------------------------------------------- the graphic store

    [Fact]
    public void TheDocumentsBulletGraphicsAreFoundUnderItsListRecord()
    {
        // Document -> List -> ProgTags -> ProgBinaryTag "___PPT9" -> BinaryTagData ->
        // ExtendedBuGraContainer -> ExtendedBuGraAtom, whose *instance* is the blip index and
        // whose content is a two-byte type word followed by a bare blip record.
        PptBulletPictures pictures = ReadPictures(Document(BuGra(0), BuGra(3)));

        pictures.IsEmpty.ShouldBeFalse();
        pictures.Graphic(0).ShouldNotBeNull().Bytes.ToArray().ShouldBe(Png);
        pictures.Graphic(3).ShouldNotBeNull().Bytes.ToArray().ShouldBe(Png);
        pictures.Graphic(1).ShouldBeNull();
        pictures.Graphic(PptExtendedParagraph.NoBulletBlip).ShouldBeNull();
    }

    [Fact]
    public void ADocumentWithNoBulletGraphicsGivesAnEmptyStore()
        => ReadPictures(Document()).IsEmpty.ShouldBeTrue();

    // ------------------------------------------------------------------------------- the size

    /// <summary>
    /// The eight sizes 26.2.4.2 states for that deck, level 1 at 155 % and level 2 at 110 %.
    /// </summary>
    /// <remarks>
    /// Read out of its own <c>--convert-to fodp</c> of the file, as
    /// <c>text:list-level-style-image/style:list-level-properties/@fo:height</c>. They are the
    /// whole evidence for <c>round(fontHeight × 0.2540 × bulletHeight)</c>: no other expression
    /// this round tried fits all eight, and the pairing of a size with a percentage is what
    /// separates the font height from the bullet's own.
    /// </remarks>
    [Theory]
    [InlineData(20, 155, 787)]
    [InlineData(16, 155, 630)]
    [InlineData(18, 155, 709)]
    [InlineData(28, 155, 1102)]
    [InlineData(5, 155, 197)]
    [InlineData(24, 110, 671)]
    [InlineData(18, 110, 503)]
    [InlineData(16, 110, 447)]
    [InlineData(20, 110, 559)]
    public void ThePictureIsAsTallAsTheReferenceStates(int fontHeight, int bulletHeight, int mm100)
    {
        PptBulletPictures pictures = ReadPictures(Document(BuGra(0)));

        SlideMarkerPicture picture = PptTextBody
            .BulletPicture(0, pictures, (ushort)fontHeight, (ushort)bulletHeight)
            .ShouldNotBeNull();

        picture.Height.ShouldBe(Length.FromMm100(mm100));

        // Square graphic, square box: the width follows the picture's own aspect ratio, which is
        // `nWidth = nHeight * aPrefSize.Width() / aPrefSize.Height()` (svdfppt.cxx:3457-3461).
        picture.Width.ShouldBe(picture.Height);
    }

    [Fact]
    public void ABlipNamingNoGraphicIsNotAPictureBulletAtAll()
    {
        // `rNumberFormat.SetNumberingType(SVX_NUM_BITMAP)` is inside
        // `if (pParaProv->GetGraphic(nBuBlip, aGraphic))` (svdfppt.cxx:3449-3465), so a dangling
        // index leaves the level's character bullet in force rather than drawing nothing.
        PptTextBody.BulletPicture(2, ReadPictures(Document(BuGra(0))), 20, 155).ShouldBeNull();
        PptTextBody.BulletPicture(0, null, 20, 155).ShouldBeNull();
    }

    // ----------------------------------------------------------------------------- the layout
    [Fact]
    public void ThePictureIsCentredOnTheLinesTextAndNoGlyphIsShapedForIt()
    {
        // `ImpCalcBulletArea`'s vertical is Top = H - TH + TH/2 - box/2 and a bitmap is drawn from
        // that top rather than lifted to a baseline (outliner.cxx:1461-1467 and :965-975). Two
        // box heights on one line therefore share a centre, whatever the line's own metrics are —
        // which is the whole of the rule, asserted without having to restate them.
        (DocRect small, int glyphs) = Marked(Length.FromMm100(300));
        (DocRect large, _) = Marked(Length.FromMm100(900));

        small.Height.ShouldBe(Length.FromMm100(300));
        large.Height.ShouldBe(Length.FromMm100(900));
        (small.Y + (small.Height / 2)).ShouldBe(large.Y + (large.Height / 2));

        // Every glyph run on the line is the paragraph's own text. A picture marker shapes
        // nothing, which is why the deck's Wingdings 2 slot never reaches a face.
        glyphs.ShouldBe(1);
    }

    [Fact]
    public void TheFirstLineClearsThePicturesWidth()
    {
        // `nStartX = max(textLeft + firstLineOffset, bulletX)` (impedit3.cxx:846-851). The
        // paragraph states no hanging indent, so the picture's own width decides.
        SlideFonts fonts = new();
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200));

        List<PlacedGlyphRun> narrow = SlideTextLayout.Place(Bulleted(Length.FromMm100(300)), area, fonts);
        List<PlacedGlyphRun> wide = SlideTextLayout.Place(Bulleted(Length.FromMm100(1200)), area, fonts);

        narrow.Count.ShouldBe(1);
        wide.Count.ShouldBe(1);
        (wide[0].Run.Origin.X - narrow[0].Run.Origin.X).ShouldBe(Length.FromMm100(900));
    }

    [Fact]
    public void AnEmptyParagraphsLineIsFlooredAtThePicturesHeightAndNotAtAFacesMetrics()
    {
        // The other half of round 110's rule. `Outliner::ImplGetBulletSize` answers
        // `pFmt->GetGraphicSize()` for a bitmap (outliner.cxx:1345-1350), so the box a blank line
        // is raised to is the picture's — not the ascent plus descent of whatever face the level's
        // bullet *character* happens to resolve to. Both boxes below are above the 10 pt line's
        // own height, so the floor is in force on both and the following paragraph moves by their
        // difference exactly.
        double tall = FollowingBaseline(Length.FromMm100(1400));
        double taller = FollowingBaseline(Length.FromMm100(1900));

        (taller - tall).ShouldBe(Length.FromMm100(500).Points, 0.001);
    }

    [Fact]
    public void APictureTooShortToReachTheLineDoesNotFloorIt()
    {
        // The guard is `nMinHeight > pTmpLine->GetHeight()` (impedit3.cxx:1976). A two-millimetre
        // bullet beside a 10 pt line changes nothing, and a reader that applied the box
        // unconditionally would shrink the line instead.
        FollowingBaseline(Length.FromMm100(100))
            .ShouldBe(FollowingBaseline(Length.FromMm100(200)), 0.001);
    }

    // ------------------------------------------------------------------------------- fixtures

    /// <summary>Places a one-line bulleted paragraph and returns its picture and its run count.</summary>
    private static (DocRect Box, int Glyphs) Marked(Length side)
    {
        List<PlacedPicture> markers = [];
        List<PlacedGlyphRun> runs = SlideTextLayout.Place(
            Bulleted(side),
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200)),
            new SlideFonts(),
            markers);

        markers.Count.ShouldBe(1);
        return (markers[0].Destination, runs.Count);
    }

    /// <summary>
    /// Where the paragraph after a floored empty one lands, in points from the block's top.
    /// </summary>
    private static double FollowingBaseline(Length side)
    {
        List<PlacedGlyphRun> runs = SlideTextLayout.Place(
            Empty(side),
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400)),
            new SlideFonts());

        runs.Count.ShouldBe(1);
        return runs[0].Run.Origin.Y.Points;
    }

    private static SlideTextBody Bulleted(Length side) => new()
    {
        Paragraphs =
        [
            new SlideParagraph(
                "Bulleted",
                [new SlideTextRun(0, 8, "Liberation Sans", Length.FromPoints(20), 400, false, Colour.Black)],
                Marker: new SlideMarker(string.Empty)
                {
                    Picture = new SlideMarkerPicture(null, null, side, side),
                }),
        ],
    };

    private static SlideTextBody Empty(Length side) => new()
    {
        Paragraphs =
        [
            new SlideParagraph(
                string.Empty,
                [new SlideTextRun(0, 0, "Liberation Sans", Length.FromPoints(10), 400, false, Colour.Black)],
                Marker: new SlideMarker(string.Empty)
                {
                    Picture = new SlideMarkerPicture(null, null, side, side),
                })
            {
                EmptyKeepsMarkerLevel = true,
            },
            new SlideParagraph(
                "After",
                [new SlideTextRun(0, 5, "Liberation Sans", Length.FromPoints(10), 400, false, Colour.Black)]),
        ],
    };

    // ------------------------------------------------------------------------- record building

    private static PptExtendedParagraphSheet Read(byte[] stream)
    {
        DffRecordBuffer buffer = new(stream);
        buffer.TryReadHeader(0, out DffRecordHeader master).ShouldBeTrue();
        return PptExtendedParagraphSheet.Read(buffer, master);
    }

    private static PptBulletPictures ReadPictures(byte[] stream)
    {
        DffRecordBuffer buffer = new(stream);
        buffer.TryReadHeader(0, out DffRecordHeader document).ShouldBeTrue();
        return PptBulletPictures.Read(buffer, document);
    }

    private static byte[] Blip(ushort index)
        => [.. Mask(0x03800000), .. U16(index), .. U16(0), .. U32(0x0001_0003), .. U32(0)];

    private static byte[] Unset() => [.. Mask(0), .. U32(0)];

    private static byte[] Mask(uint value) => U32(value);

    private static byte[] U16(ushort value) => [(byte)value, (byte)(value >> 8)];

    private static byte[] U32(uint value)
        => [(byte)value, (byte)(value >> 8), (byte)(value >> 16), (byte)(value >> 24)];

    private static byte[] MainMaster(params (PptTextKind Kind, byte[][] Levels)[] atoms)
    {
        List<byte> data = [];
        foreach ((PptTextKind kind, byte[][] levels) in atoms)
        {
            List<byte> payload = [.. U16((ushort)levels.Length)];
            foreach (byte[] level in levels) payload.AddRange(level);

            Record(
                data, PptRecordTypes.ExtendedParagraphMasterAtom, container: false,
                (ushort)kind, [.. payload]);
        }

        return [.. Tagged(PptRecordTypes.MainMaster, [.. data])];
    }

    private static byte[] Document(params byte[][] graphics)
    {
        List<byte> atoms = [];
        foreach (byte[] graphic in graphics) atoms.AddRange(graphic);

        List<byte> container = [];
        if (graphics.Length > 0)
        {
            Record(
                container, PptRecordTypes.ExtendedBuGraContainer, container: true, 0, [.. atoms]);
        }

        byte[] list = Tagged(PptRecordTypes.List, [.. container]);

        List<byte> document = [];
        Record(document, PptRecordTypes.Document, container: true, 0, list);
        return [.. document];
    }

    /// <summary>One <c>ExtendedBuGraAtom</c>: a type word, then the blip record itself.</summary>
    private static byte[] BuGra(ushort instance)
    {
        List<byte> payload = [.. U16(1542)];

        List<byte> blip = [];
        List<byte> content = [.. new byte[16], 0xFF, .. Png];
        Record(blip, 0xF01E, container: false, instance: 0x6E0, [.. content]);
        payload.AddRange(blip);

        List<byte> atom = [];
        Record(atom, PptRecordTypes.ExtendedBuGraAtom, container: false, instance, [.. payload]);
        return [.. atom];
    }

    /// <summary>A container carrying the <c>___PPT9</c> tag, with the given records inside it.</summary>
    private static byte[] Tagged(ushort type, byte[] records)
    {
        List<byte> tagData = [];
        Record(tagData, PptRecordTypes.BinaryTagData, container: false, 0, records);

        List<byte> name = [];
        List<byte> characters = [];
        foreach (char c in "___PPT9") characters.AddRange(U16(c));
        Record(name, PptRecordTypes.CString, container: false, 0, [.. characters]);

        List<byte> binaryTag = [];
        Record(
            binaryTag, PptRecordTypes.ProgBinaryTag, container: true, 0,
            [.. name, .. tagData]);

        List<byte> tags = [];
        Record(tags, PptRecordTypes.ProgTags, container: true, 0, [.. binaryTag]);

        List<byte> outer = [];
        Record(outer, type, container: true, 0, [.. tags]);
        return [.. outer];
    }

    private static void Record(
        List<byte> into, ushort type, bool container, ushort instance, byte[] payload)
    {
        ushort versionAndInstance = (ushort)((instance << 4) | (container ? 0x0F : 0x02));
        into.AddRange(U16(versionAndInstance));
        into.AddRange(U16(type));
        into.AddRange(U32((uint)payload.Length));
        into.AddRange(payload);
    }
}
