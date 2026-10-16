import sys, os
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *   # reuses the loaded tables and prints the legs again
print()

def i(x):
    try: return int(x)
    except: return 0

keys = sorted(ref_F)
# --- headline error figures for leg E
pe = sum(abs(i(ours_head_F[k][0]) - i(ref_F[k][0])) for k in keys)
we = sum(abs(i(ours_head_F[k][1]) - i(ref_F[k][1])) for k in keys)
exact = sum(1 for k in keys if ours_head_F[k][0] == ref_F[k][0])
print(f"leg E: abs page error {pe}, exact page counts {exact}/200, abs word error {we}")
from collections import Counter
c = Counter(verdict(ours_head_F[k][0], ref_F[k][0], ours_head_F[k][1], ref_F[k][1], ours_head_F[k][3]) for k in keys)
print("verdict breakdown:", dict(c))
# and the predecessor's environment, for comparison
pe_n = sum(abs(i(ours_head_N[k][0]) - i(ref_N[k][0])) for k in keys)
we_n = sum(abs(i(ours_head_N[k][1]) - i(ref_N[k][1])) for k in keys)
ex_n = sum(1 for k in keys if ours_head_N[k][0] == ref_N[k][0])
print(f"leg D: abs page error {pe_n}, exact page counts {ex_n}/200, abs word error {we_n}")
print()

# --- how much does OURS move with the font set (same code, same binary)?
mv = [(i(ours_head_F[k][0]) - i(ours_head_N[k][0]), k) for k in keys]
moved = [x for x in mv if x[0] != 0]
print(f"OURS page counts changed by installing DejaVu: {len(moved)} of 200, "
      f"total |d| {sum(abs(d) for d,_ in moved)}, down {sum(1 for d,_ in moved if d<0)}, up {sum(1 for d,_ in moved if d>0)}")
for d,k in sorted(moved)[:6] + sorted(moved)[-4:]:
    print(f"   {d:+5d}  {i(ours_head_N[k][0]):5d} -> {i(ours_head_F[k][0]):5d}  {os.path.basename(k)}")
wv = [(i(ours_head_F[k][1]) - i(ours_head_N[k][1]), k) for k in keys]
wmoved = [x for x in wv if x[0] != 0]
print(f"OURS word counts changed: {len(wmoved)} of 200, total |d| {sum(abs(d) for d,_ in wmoved)}")

# --- the reference's own font movement (both on disk)
rv = [(i(ref_F[k][0]) - i(ref_N[k][0]), k) for k in keys]
rmoved = [x for x in rv if x[0] != 0]
print(f"REF page counts changed by installing DejaVu: {len(rmoved)} of 200, "
      f"total |d| {sum(abs(d) for d,_ in rmoved)}, down {sum(1 for d,_ in rmoved if d<0)}, up {sum(1 for d,_ in rmoved if d>0)}")
rwv = [(i(ref_F[k][1]) - i(ref_N[k][1]), k) for k in keys]
print(f"REF word counts changed: {sum(1 for d,_ in rwv if d)} of 200, total |d| {sum(abs(d) for d,_ in rwv)}")
print()

# --- the three outliers, every column available
out = ["A_320.doc", "AC-150-5370-10G-updated-201604.docx", "150-5370-10H.docx"]
print(f"{'document':44s} {'ours47/ref24':>13s} {'oursH-f/ref-f':>14s} {'oursH+f/ref+f':>14s}")
for name in out:
    k = [x for x in keys if os.path.basename(x) == name][0]
    print(f"{name:44s} {ours_r47[k][0]+'/'+ref_24[k][0]:>13s} "
          f"{ours_head_N[k][0]+'/'+ref_N[k][0]:>14s} {ours_head_F[k][0]+'/'+ref_F[k][0]:>14s}"
          f"   verdict={verdict(ours_head_F[k][0], ref_F[k][0], ours_head_F[k][1], ref_F[k][1], ours_head_F[k][3])}")
print()

# --- top absolute page errors now
err = sorted(((abs(i(ours_head_F[k][0])-i(ref_F[k][0])), k) for k in keys), reverse=True)[:10]
print("top page errors, leg E:")
for e,k in err:
    print(f"   {e:4d}  {ours_head_F[k][0]:>4s}/{ref_F[k][0]:<4s}  was {ours_head_N[k][0]}/{ref_N[k][0]} (-f), {ours_r47[k][0]}/{ref_24[k][0]} (r47/24.2)  {os.path.basename(k)}")
print(f"top 8 carry {sum(e for e,_ in err[:8])} of {pe}")
