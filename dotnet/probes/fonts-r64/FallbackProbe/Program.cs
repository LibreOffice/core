using XRay.Office.Text.Fonts;

// What this machine's own font set answers, asked four ways. Written to establish the *shape* of
// the defect before any rendering was done; the rendered probes that decide it are `gen-generic.py`
// and `gen-fallback.py` beside this.
//
//   ranks                      the merged fontconfig preference order, and which entries are installed
//   installed-list             which of LibreOffice's fixed glyph-fallback families are installed here
//   resolve <family>...        what a bare request for a family answers under each declared class
//   <hex> <hex> ...            per code point: what the fixed list answers, what the fontconfig-order
//                              stage answers, and whether they differ
var index = SystemFontIndex.Build();
var prefs = FontconfigPreferences.Machine;

if (args.Length > 0 && args[0] == "ranks")
{
    int i = 0;
    foreach (string f in prefs.InOrder)
    {
        Console.WriteLine($"{i++}\t{f}\t{(index.Best(f, 400, false) is null ? "" : "installed")}");
    }
    return;
}

if (args.Length > 1 && args[0] == "resolve")
{
    foreach (string fam in args.Skip(1))
    {
        foreach (FontFamilyClass cls in new[]
                 { FontFamilyClass.Unknown, FontFamilyClass.Serif, FontFamilyClass.SansSerif, FontFamilyClass.Fixed })
        {
            var r = new SystemFontResolver(index).Resolve(
                new FontRequest { FamilyName = fam, Weight = 400, IsItalic = false, DeclaredClass = cls });
            Console.WriteLine($"{fam}\t{cls}\t{r.FamilyName}");
        }
    }
    return;
}

if (args.Length > 0 && args[0] == "installed-list")
{
    foreach (string f in GlyphFallbackFamilies.InOrder)
    {
        if (index.Best(f, 400, false) is { } b) Console.WriteLine($"{f}\t{b.FamilyName}");
    }
    return;
}

Console.WriteLine("cp\tlist\tfcorder\tdiffers");
foreach (string arg in args)
{
    int cp = Convert.ToInt32(arg, 16);
    string list = "-";
    foreach (string family in GlyphFallbackFamilies.InOrder)
    {
        if (index.Best(family, 400, false) is not { } cand) continue;
        var face = OpenTypeFace.ReadFile(SplitPath(cand.FaceKey), SplitIndex(cand.FaceKey));
        if (face.HasGlyphFor(cp)) { list = face.FamilyName ?? family; break; }
    }

    string fc = "-";
    foreach (var cand in index.Faces
        .OrderBy(f => prefs.RankOf(f.FamilyName))
        .ThenBy(f => f.IsItalic ? 1 : 0)
        .ThenBy(f => Math.Abs(f.Weight - 400))
        .ThenBy(f => f.FamilyName, StringComparer.Ordinal))
    {
        OpenTypeFace face;
        try { face = OpenTypeFace.ReadFile(SplitPath(cand.FaceKey), SplitIndex(cand.FaceKey)); }
        catch { continue; }
        if (face.HasGlyphFor(cp)) { fc = face.FamilyName ?? cand.FamilyName; break; }
    }

    Console.WriteLine($"U+{cp:X4}\t{list}\t{fc}\t{(list == fc ? "" : "DIFFERS")}");
}

static string SplitPath(string key)
{
    int h = key.LastIndexOf('#');
    return h < 0 ? key : key[..h];
}

static int SplitIndex(string key)
{
    int h = key.LastIndexOf('#');
    return h < 0 ? 0 : int.Parse(key[(h + 1)..]);
}
