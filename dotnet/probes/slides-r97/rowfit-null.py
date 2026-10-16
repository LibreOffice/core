#!/usr/bin/env python3
"""How often an ARBITRARY pair of integer sizes admits `autofit-rows.txt`'s fit.

`autofit-rows.txt` reports that 18 of the 19 dominant-size rows admit one integer stated size
and two rows of `constScaleLevels` (`editeng/source/editeng/impedit3.cxx`:286-300) giving our
drawn size and the reference's. That is only evidence if a pair that is NOT an autofit rarely
admits one. Eleven distinct scale levels and a free integer stated size are a lot of freedom;
this measures exactly how much.
"""
LEVELS = [1.000, 0.925, 0.850, 0.775, 0.700, 0.625, 0.550, 0.475, 0.400, 0.325, 0.250]

def admits(a, b, smax=96):
    for s in range(6, smax + 1):
        if any(round(s * l) == a for l in LEVELS) and any(round(s * l) == b for l in LEVELS):
            return True
    return False

for label, keep in (('any pair', lambda a, b: True),
                    ('pairs within 4 pt', lambda a, b: abs(a - b) <= 4),
                    ('pairs within 2 pt', lambda a, b: abs(a - b) <= 2)):
    tot = ok = 0
    for a in range(8, 49):
        for b in range(8, 49):
            if a == b or not keep(a, b): continue
            tot += 1; ok += admits(a, b)
    print(f'integer sizes 8..48, {label:20s}: {ok:5d} / {tot:5d} = {100*ok/tot:5.1f}% admit a (size, row, row) triple')
print()
print('The 19 rows are all within 3 pt of each other bar two. Against a null of about 71%,')
print('"18 of 19 admit one" is not evidence for the autofit reading -- the fit is close to')
print('vacuous and autofit-rows.txt should be read as a description, not as a measurement.')
