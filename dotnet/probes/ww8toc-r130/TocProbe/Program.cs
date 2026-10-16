// Dumps a WW8 document's stylesheet and the character-style index in force over a character range,
// so that "which style makes the contents entries blue" is read off the file rather than guessed.
using System.Buffers.Binary;
using XRay.Office.Containers;
using XRay.Office.Containers.Ole2;
using XRay.Office.WordProcessing.Ww8;

if (args.Length > 3 && args[1] == "--frames")
{
    Frames.Dump(args[0], int.Parse(args[2]), int.Parse(args[3]));
    return;
}

if (args.Length > 1 && args[1] == "--toggles")
{
    Console.WriteLine("document\tchpx_with_style\tchpx_without_style\tcharacters");
    foreach (string one in args[2..])
    {
        try { Toggles.Row(one, ReadAll(one)); }
        catch (Exception e) { Console.Error.WriteLine($"{one}: {e.GetType().Name}"); }
    }

    return;
}

if (args.Length > 1 && args[1] == "--census")
{
    Console.WriteLine("document\ttoc_fields\tchars_in_result\thyperlink_style\tother_style\tdecorated_before");
    foreach (string one in args[2..])
    {
        try { Census.Row(one, ReadAll(one)); }
        catch (Exception e) { Console.Error.WriteLine($"{one}: {e.GetType().Name}"); }
    }

    return;
}

string path = args[0];
using FileStream fs = File.OpenRead(path);
CompoundFile file = CompoundFile.Open(fs, leaveOpen: true);
byte[] wordDocument = Read(file, "WordDocument")!;
Ww8Fib fib = Ww8Fib.Parse(wordDocument);
byte[] table = Read(file, fib.UsesTable1Stream ? "1Table" : "0Table") ?? [];

// STSH
int stshOff = (int)fib.FileOffset(Ww8FibTable.StyleSheet);
int stshLen = (int)fib.Length(Ww8FibTable.StyleSheet);
Ww8StyleSheet styles = Ww8StyleSheet.Parse(table.AsSpan(stshOff, stshLen));
Console.WriteLine($"styles: {styles.Styles.Count}");
for (int i = 0; i < styles.Styles.Count; i++)
{
    Ww8Style s = styles.Styles[i];
    if (s.Kind == 0) continue;
    Console.WriteLine($"  [{i}] kind={s.Kind} base={s.BaseIndex} name={s.Name} chpx={Convert.ToHexString(s.CharacterProperties.Span)}");
}

Fields.Dump(wordDocument, table, fib, styles);
if (args.Length > 1) Runs.Dump(wordDocument, table, fib, styles, args[1]);

static (byte[] Word, byte[] Table, Ww8Fib Fib, Ww8StyleSheet Styles) ReadAll(string one)
{
    using FileStream stream = File.OpenRead(one);
    CompoundFile cf = CompoundFile.Open(stream, leaveOpen: true);
    byte[] word = Read(cf, "WordDocument")!;
    Ww8Fib f = Ww8Fib.Parse(word);
    byte[] tbl = Read(cf, f.UsesTable1Stream ? "1Table" : "0Table") ?? [];
    Ww8StyleSheet st = Ww8StyleSheet.Parse(
        tbl.AsSpan((int)f.FileOffset(Ww8FibTable.StyleSheet), (int)f.Length(Ww8FibTable.StyleSheet)));
    return (word, tbl, f, st);
}

static byte[]? Read(CompoundFile package, string name)
{
    IPackagePart? part = package.GetPart(name);
    if (part is null) return null;
    using Stream s = part.Open();
    using MemoryStream ms = new();
    s.CopyTo(ms);
    return ms.ToArray();
}
