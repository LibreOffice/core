# -*- encoding: UTF-8 -*-
from __future__ import unicode_literals
import uno
import re
import sys
import os
import traceback
from com.sun.star.text.TextMarkupType import PROOFREADING
from com.sun.star.beans import PropertyValue

pkg = "hu_HU"
lang = "hu_HU"
locales = {'hu-HU': ['hu', 'HU', '']}
version = "1.6.4"
author = "László Németh"
name = "Lightproof grammar checker (magyar)"

import lightproof_handler_hu_HU

# loaded rules (check for Update mechanism of the editor)
try:
    langrule
except NameError:
    langrule = {}

# ignored rules
ignore = {}

# cache for morphogical analyses
analyses = {}
stems = {}
suggestions = {}

# assign Calc functions
calcfunc = None

# check settings
def option(lang, opt):
    return lightproof_handler_hu_HU.get_option(lang.Language + "_" + lang.Country, opt)

# filtering affix fields (ds, is, ts etc.)
def onlymorph(st):
    if st is not None:
        st = re.sub(r"^.*(st:|po:)", r"\\1", st) # keep last word part
        st = re.sub(r"\\b(?=[dit][sp]:)","@", st) # and its affixes
        st = re.sub(r"(?<!@)\\b\w\w:\w+","", st).replace('@','').strip()
    return st

# if the pattern matches all analyses of the input word, 
# return the last matched substring
def _morph(rLoc, word, pattern, all, onlyaffix):
    global analyses
    if not word:
        return None
    if word not in analyses:
        x = spellchecker.spell(u"<?xml?><query type='analyze'><word>" + word + "</word></query>", rLoc, ())
        if not x:
            return None
        t = x.getAlternatives()
        if not t:
            if not analyses: # fix synchronization problem (missing alternatives with unloaded dictionary)
                return None
            t = [""]
        analyses[word] = t[0].split("</a>")[:-1]
    a = analyses[word]
    result = None
    p = re.compile(pattern)
    for i in a:
        if onlyaffix:
            i = onlymorph(i)
        result = p.search(i)
        if result:
            result = result.group(0)
            if not all:
                return result
        elif all:
            return None
    return result

def morph(rLoc, word, pattern, all=True):
    return _morph(rLoc, word, pattern, all, False)

def affix(rLoc, word, pattern, all=True):
    return _morph(rLoc, word, pattern, all, True)

def spell(rLoc, word):
    if not word:
        return None
    return spellchecker.isValid(word, rLoc, ())

# get the tuple of the stem of the word or an empty array
def stem(rLoc, word):
    global stems
    if not word:
        return []
    if word not in stems:
        x = spellchecker.spell(u"<?xml?><query type='stem'><word>" + word + "</word></query>", rLoc, ())
        if not x:
            return []
        t = x.getAlternatives()
        if not t:
            t = []
        stems[word] = list(t)
    return stems[word]

# get the tuple of the morphological generation of a word or an empty array
def generate(rLoc, word, example):
    if not word:
        return []
    x = spellchecker.spell(u"<?xml?><query type='generate'><word>" + word + "</word><word>" + example + "</word></query>", rLoc, ())
    if not x:
        return []
    t = x.getAlternatives()
    if not t:
        t = []
    return list(t)

# get suggestions
def suggest(rLoc, word):
    global suggestions
    if not word:
        return word
    if word not in suggestions:
        x = spellchecker.spell("_" + word, rLoc, ())
        if not x:
            return word
        t = x.getAlternatives()
        suggestions[word] = "|".join(t)
    return suggestions[word]

# get the nth word of the input string or None
def word(s, n):
    a = re.match("(?u)( [-.\\w%%]+){" + str(n-1) + "}( [-.\\w%%]+)", s)
    if not a:
        return ''
    return a.group(2)[1:]

# get the (-)nth word of the input string or None
def wordmin(s, n):
    a = re.search("(?u)([-.\\w%%]+ )([-.\\w%%]+ ){" + str(n-1) + "}$", s)
    if not a:
        return ''
    return a.group(1)[:-1]

def calc(funcname, par):
    global calcfunc
    global SMGR
    if calcfunc is None:
        calcfunc = SMGR.createInstance( "com.sun.star.sheet.FunctionAccess")
        if calcfunc is None:
                return None
    return calcfunc.callFunction(funcname, par)

def proofread( nDocId, TEXT, LOCALE, nStartOfSentencePos, nSuggestedSentenceEndPos, rProperties ):
    global ignore
    aErrs = []
    s = TEXT[nStartOfSentencePos:nSuggestedSentenceEndPos]
    for i in get_rule(LOCALE).dic:
        # 0: regex,  1: replacement,  2: message,  3: condition,  4: ngroup,  (5: oldline),  6: case sensitive ?
        if i[0] and str(i[0]) not in ignore:
            for m in i[0].finditer(s):
                try:
                    if not i[3] or eval(i[3]):
                        aErr = uno.createUnoStruct( "com.sun.star.linguistic2.SingleProofreadingError" )
                        aErr.nErrorStart        = nStartOfSentencePos + m.start(i[4]) # nStartOfSentencePos
                        aErr.nErrorLength       = m.end(i[4]) - m.start(i[4])
                        aErr.nErrorType         = PROOFREADING
                        aErr.aRuleIdentifier    = str(i[0])
                        iscap = (i[-1] and m.group(i[4])[0:1].isupper())
                        if i[1][0:1] == "=":
                            aErr.aSuggestions = tuple(cap(eval(i[1][1:]).replace('|', "\n").split("\n"), iscap, LOCALE))
                        elif i[1] == "_":
                            aErr.aSuggestions = ()
                        else:
                            aErr.aSuggestions = tuple(cap(m.expand(i[1]).replace('|', "\n").split("\n"), iscap, LOCALE))
                        comment = i[2]
                        if comment[0:1] == "=":
                            comment = eval(comment[1:])
                        else:
                            comment = m.expand(comment)
                        aErr.aShortComment      = comment.replace('|', '\n').replace('\\n', '\n').split("\n")[0].strip()
                        aErr.aFullComment       = comment.replace('|', '\n').replace('\\n', '\n').split("\n")[-1].strip()
                        if "://" in aErr.aFullComment:
                            p = PropertyValue()
                            p.Name = "FullCommentURL"
                            p.Value = aErr.aFullComment
                            aErr.aFullComment = aErr.aShortComment
                            aErr.aProperties        = (p,)
                        else:
                            aErr.aProperties        = ()
                        aErrs = aErrs + [aErr]
                except Exception as e:
                    if len(i) == 7:
                        raise Exception(str(e), i[5])
                    raise

    return tuple(aErrs)

def cap(a, iscap, rLoc):
    for i in range(0, len(a)):
        if a[i][0:6] == "!CASE!":
            a[i] = a[i][6:]
        elif iscap:
            if a[i][0:1] == "i":
                if rLoc.Language == "tr" or rLoc.Language == "az":
                    a[i] = u"\u0130" + a[i][1:]
                elif a[i][1:2] == "j" and rLoc.Language == "nl":
                    a[i] = "IJ" + a[i][2:]
                else:
                    a[i] = "I" + a[i][1:]
            else:
                a[i] = a[i].capitalize()
    return a

def compile_rules(dic):
    # compile regular expressions
    for i in dic:
        try:
            if re.compile("[(][?]iu[)]").match(i[0]):
                i += [True]
                i[0] = re.sub("[(][?]iu[)]", "(?u)", i[0])
            else:
                i += [False]
            i[0] = re.compile(i[0])
        except Exception:
            if 'PYUNO_LOGLEVEL' in os.environ:
                print("Lightproof: bad regular expression: " + str(traceback.format_exc()))
            i[0] = None

def get_rule(loc):
    try:
        return langrule[pkg]
    except Exception:
        langrule[pkg] = __import__("lightproof_" + pkg)
        compile_rules(langrule[pkg].dic)
    return langrule[pkg]

def get_path():
    return os.path.join(os.path.dirname(sys.modules[__name__].__file__), __name__ + ".py")

# [code]


abbrev=re.compile(r"(?i)\\b([a-z\xf6\xfc\xf3\u0151\xfa\xe9\xe1\u0171\xed\xd6\xdc\xd3\u0150\xda\xc9\xc1\u0170\xcd]|\xc1e|\xc1ht|AkH|al|\xe1lt|\xe1pr|aug|Avtv|bek|Bp|br|bt|Btk|cca|ci(i|ii|v|x)?|cl(i|ii|iii|iv|ix|v|vi|vii|viii|x|xi|xii|xiii|xiv|xix|xv|xvi|xvii|xviii|xx|xxi|xxii|xxiii|xxiv|xxix|xxv|xxvi|xxvii|xxviii|xxx|xxxi|xxxii|xxxiii|xxxiv|xxxix|xxxv|xxxvi|xxxvii|xxxviii)?|Co|cv(i|ii|iii)?|cx(c|ci|cii|ciii|civ|cix|cv|cvi|cvii|cviii|i|ii|iii|iv|ix|l|li|lii|liii|liv|lix|lv|lvi|lvii|lviii|v|vi|vii|viii|x|xi|xii|xiii|xiv|xix|xv|xvi|xvii|xviii|xx|xxi|xxii|xxiii|xxiv|xxix|xxv|xxvi|xxvii|xxviii)?|cs|Csjt|Cstv|cs\xfct|dec|dk|dny|dr|du|dz(s)?|egy|\xe9k|\xc9Ksz|em|\xe9ny|\xc9pt|\xe9rk|etc|Etv|e\xfc|ev|\xe9vf|febr|felv|Flt|ford|f\u0151isk|fsz(la|t)?|Ftv|gimn|g\xf6r|gr|Gt|gy|Gyvt|habil|hg|hiv|Hjt|honv|Hpt|hrsz|hsz|Hszt|htb|id|ifj|ig(h)?|ii(i)?|ill|Inc|ind|isk|iv|ix|izr|jan|jegyz|j\xfal|j\xfan|kat|kb|Kbt|ker|kft|kgy|kht|kir|kiv|Kjt|kk(t)?|koll|korm|k\xf6v|kp|Kr|krt|Kt(v)?|ld|li(i|ii|v|x)?|Ltd|ltp|Ltv|luth|lv(i|ii|iii)?|lx(i|ii|iii|iv|ix|v|vi|vii|viii|x|xi|xii|xiii|xiv|xix|xv|xvi|xvii|xviii|xx|xxi|xxii|xxiii|xxiv|xxix|xxv|xxvi|xxvii|xxviii)?|ly|m\xe1j|m\xe1rc|mat|max|mb|megh|megj|MHSz|min|mk|Mo|Mt|NB|nov|ny(\xe1)?|Nyilv|nyrt|okl|okt|olv|op|orsz|ort|ov(h)?|\xf6ssz|\xd6tv|\xf6zv|Pf|pl(d)?|prof|prot|Ptk|pu|ref|rk(p)?|r\xf3m|r\xf6v|rt|sgt|spec|stb|sz(ept|erk)?|Szjt|szoc|Szt(v)?|sz\xfcl|Tbj|tc|tel|tkp|tszf|tvr|ty|ua|ui|\xfam|\xfan|uo|Ve|Vhr|vi(i|ii)?|v\xf6|vsz|Vt(v)?|xc(i|ii|iii|iv|ix|v|vi|vii|viii)?|xi(i|ii|v|x)?|xl(i|ii|iii|iv|ix|v|vi|vii|viii)?|xv(i|ii|iii)?|xx(i|ii|iii|iv|ix|v|vi|vii|viii|x|xi|xii|xiii|xiv|xix|xv|xvi|xvii|xviii)?|zrt)\\.")

# pattern for paragraph checking
paralcap = re.compile(u"(?u)^[a-z\xf6\xfc\xf3\u0151\xfa\xe9\xe1\u0171\xed].*[.?!] [A-Z\xd6\xdc\xd3\u0150\xda\xc9\xc1\u0170\xcd].*[.?!][)\u201d]?$")


foreign=["ab ovo", "ars poetica", "casus belli", "categoricus imperativus",
    "coming out", "cash flow", "circulus vitiosus", "corpus delicti", "de facto", "de jure",
    "delirium tremens", "doctor honoris causa", "et cetera", "fait accompli", "fixa idea",
    "horribile dictu", "hot dog", "in flagranti", "in medias res", "in memoriam", "in vitro",
    "in vivo", "magna cum laude", "mea culpa", "memento mori", "mountain bike", "nota bene",
    "persona non grata", "plein air", "pro bono", "salto mortale", "status quo", "tabula rasa",
    "terminus technicus", "vice versa", "vis maior"]

def suggest_foreign(word):
    return "\n".join([i for i in foreign if word == i.split(" ")[0]])


def measurement(mnum, min, mout, mstr):
    m = calc("CONVERT", (float(mnum.replace(",", ".").replace(u"\u2212", "-")), min, mout))
    a = list(set([str(calc("ROUND", (m, 0)))[:-2], str(calc("ROUND", (m, 1))), str(calc("ROUND", (m, 2))), str(m)])) # remove duplicated rounded items
    a.sort(key=lambda x: len(x)) # sort by string length
    return (mstr + "|").join(a).replace(".", ",").replace("-", u"\u2212") + mstr


