import olefile, struct, sys, shutil, os
# Zero the family nibble of every FontEntityAtom in the PowerPoint Document stream, in place.
src, dst = sys.argv[1], sys.argv[2]
shutil.copyfile(src, dst)
ole = olefile.OleFileIO(dst, write_mode=True)
st = [s for s in ole.listdir() if s[-1] == 'PowerPoint Document']
d = bytearray(ole.openstream(st[0]).read())
n = 0; i = 0
while True:
    j = d.find(b'\xb7\x0f', i)
    if j < 0: break
    i = j + 1
    if j + 6 + 68 > len(d): continue
    if struct.unpack('<I', d[j+2:j+6])[0] != 68: continue
    off = j + 6 + 67
    if d[off] & 0xF0:
        d[off] &= 0x0F; n += 1
ole.write_stream('PowerPoint Document', bytes(d))
ole.close()
print(n)
