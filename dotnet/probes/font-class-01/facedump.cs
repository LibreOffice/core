using XRay.Office.Text.Fonts;

// Dumps, for every family named on stdin's list: what LibreOffice's own table calls its shape, and
// what this resolver answers for a bare request naming it. Joined against `lo-faces-26.tsv`, which
// is the same question asked of the running 26.2.4.2, it is the whole measurement this round rests
// on. See dotnet/probes/font-class-01/results.md.
string listPath = args[0];
var index = SystemFontIndex.Build();
var resolver = new SystemFontResolver(index);
Console.WriteLine("family\ttable_class\tresolved");
foreach (string raw in File.ReadLines(listPath))
{
    string fam = raw.TrimEnd('\r', '\n');
    if (fam.Length == 0) continue;
    var r = resolver.Resolve(new FontRequest { FamilyName = fam, Weight = 400, IsItalic = false });
    Console.WriteLine($"{fam}\t{FontSubstitutions.ClassOf(fam)}\t{r.FamilyName}");
}
