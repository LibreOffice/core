# Round forty — sheets: the page-split cluster

Base `9b1429040`, checked with `git log --oneline -1` before anything was measured.

## Baseline, reproduced

`base-whole-track.tsv`, 171 rows, no duplicate path, no `ref-failed`, no `ours-failed`:

    155/171 matches   abs page error 73   exact page counts 161   abs word error 27163

Every one of the four briefed figures reproduces to the digit.

Per batch: 001–009 89/89, 010 8/10, 011 9/10, 012 9/10, 013 8/10, 014 9/10, 015 7/9,
016 7/9, 017 6/10, 018 3/4.

## The cluster the gate can see

Ten of the sixteen residual failures have a wrong page count. Ours minus the reference:

| document | pages | Δ |
|---|---|---:|
| `FAA-2019-0995-0002_attachment_2.xlsx` | 32/33 | −1 |
| `CSJU List of Recipients of funds 2013-2020.xlsx` | 97/96 | +1 |
| `FY2018_Q4_UAS_Sightings.xlsx` | 304/302 | +2 |
| `grants-2005.xls` | 219/220 | −1 |
| `ans_mappings_of_eccairs_terms.xlsx` | 190/191 | −1 |
| `aircraft_analysis_2016-04-27.xls` | 44/46 | −2 |
| `7-memento-2015-transports-aeriens-b.xls` | 190/191 | −1 |
| `SIL_TDB648.xlsx` | 89/88 | +1 |
| `orbus_togaf_tool_csq.xls` | 33/75 | −42 |
| `ODs-February-2022-Airbus-Commercial-Aircraft.xlsx` | 154/175 | −21 |

The other six fail on words alone.
