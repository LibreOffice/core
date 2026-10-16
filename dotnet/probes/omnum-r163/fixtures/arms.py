import make

LEVELS = [(927,360,'Heading1','%1.'), (2061,360,'Heading2','%1.%2'),
          (862,720,'Heading3','%1.%2.%3'), (3413,720,'Heading4','%1.%2.%3.%4')]
NUM = make.numbering(LEVELS)

IND851 = '<w:ind w:left="851" w:hanging="851"/>'

# Arm A: faithful replica of the real document's style chain (every heading style states w:ind).
A_STYLES = make.styles([
    ('Heading1','heading 1',None,None,1,IND851,0),
    ('Heading2','heading 2','Heading1',1,None,IND851,1),
    ('Heading3','heading 3','Heading2',2,None,IND851,2),
    ('Heading4','heading 4','Heading3',3,None,IND851,3),
])
DOC = make.document([('Heading1',None,'Alpha'),('Heading2',None,'Bravo'),
                     ('Heading3',None,'Charlie'),('Heading4',None,'Delta')])
make.build('arm-a-styles-have-ind.docx', NUM, A_STYLES, DOC)

# Arm B: no w:ind on any heading style at all.
B_STYLES = make.styles([
    ('Heading1','heading 1',None,None,1,None,0),
    ('Heading2','heading 2','Heading1',1,None,None,1),
    ('Heading3','heading 3','Heading2',2,None,None,2),
    ('Heading4','heading 4','Heading3',3,None,None,3),
])
make.build('arm-b-styles-no-ind.docx', NUM, B_STYLES, DOC)

# Arm C: replica, but each paragraph carries a hard w:ind of its own.
DOC_C = make.document([('Heading1',IND851,'Alpha'),('Heading2',IND851,'Bravo'),
                       ('Heading3',IND851,'Charlie'),('Heading4',IND851,'Delta')])
make.build('arm-c-paragraph-ind.docx', NUM, A_STYLES, DOC_C)

# Arm D: like A but with NO settings.xml -- the trap the corpus skill warns about.
make.build('arm-d-no-settings.docx', NUM, A_STYLES, DOC, with_settings=False)

# Arm E: only the TOP style (Heading1, the one carrying numId) states w:ind.
E_STYLES = make.styles([
    ('Heading1','heading 1',None,None,1,IND851,0),
    ('Heading2','heading 2','Heading1',1,None,None,1),
    ('Heading3','heading 3','Heading2',2,None,None,2),
    ('Heading4','heading 4','Heading3',3,None,None,3),
])
make.build('arm-e-only-top-ind.docx', NUM, E_STYLES, DOC)
