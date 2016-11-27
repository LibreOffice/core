# -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from __future__ import unicode_literals
import sys, os, uno, unohelper
import re, random, traceback, itertools
import threading, time as __time__

try:
    unicode
    next = lambda l: l.next() # python 2
except:
    unicode, long = str, int # support python 3

urebootstrap = os.environ["URE_BOOTSTRAP"]
if "vnd.sun.star.pathname" in urebootstrap:
    __lngpath__ = re.sub(r"^vnd.sun.star.pathname:(.*)program(/|\\)fundamental([.]ini|rc)$", "\\1", urebootstrap)
else:
    # A way to know if we use MacOs
    if "Resources" in urebootstrap:
        __lngpath__ = unohelper.fileUrlToSystemPath(re.sub("fundamentalrc$", "", urebootstrap))
    else:
        __lngpath__ = unohelper.fileUrlToSystemPath(re.sub("program/(fundamental.ini|fundamentalrc)$", "share", urebootstrap))
__lngpath__ = __lngpath__ + "/Scripts/python/LibreLogo/".replace("/", os.sep)
__translang__ = "am|ca|cs|de|dk|el|en|eo|es|et|fr|hu|it|ja|nl|no|pl|pt|ru|se|sl" # FIXME supported languages for language guessing, expand this list, according to the localizations
__lng__ = {}
__docs__ = {}
__prevcode__ = None
__prevlang__ = None
__prevcompiledcode__ = None
__thread__ = None
__lock__ = threading.Lock()
__halt__ = False
__compiled__ = ""
__group__ = 0
__groupstack__ = []
__grouplefthang__ = 0
__comp__ = {}
__strings__ = []
__colors__ = {}
__COLORS__ = ['BLACK', 0x000000], ['SILVER', 0xc0c0c0], ['GRAY', 0x808080], \
    ['WHITE', 0xffffff], ['MAROON', 0x800000], ['RED', 0xff0000], \
    ['PURPLE', 0x800080], ['FUCHSIA', 0xff00ff], ['GREEN', 0x008000], \
    ['LIME', 0x00ff00], ['OLIVE', 0x808000], ['YELLOW', 0xffff00], \
    ['NAVY', 0x000080], ['BLUE', 0x0000ff], ['TEAL', 0x008080], \
    ['AQUA', 0x00ffff], ['PINK', 0xffc0cb], ['TOMATO', 0xff6347], \
    ['ORANGE', 0xffa500], ['GOLD', 0xffd700], ['VIOLET', 0x9400d3], \
    ['SKYBLUE', 0x87ceeb], ['CHOCOLATE', 0xd2691e], ['BROWN', 0xa52a2a], \
    ['INVISIBLE', 0xffffffff]
__NORMCOLORS__ = [[[255, 255, 0], 0, -11, 1, -11],
    [[255, 128, 0], 1, 116, 1, -33], [[255, 0, 0], 1, 95, 2, 42],
    [[255, 0, 255], 2, -213, 0, -106], [[0, 0, 255], 0, 148, 1, 127],
    [[0, 255, 255], 1, -128, 2, -63], [[0, 255, 0], 2, 192, 0, 244]]
__STRCONST__ = [i[0] for i in __COLORS__] + ['NONE', 'BEVEL', 'MITER', 'ROUNDED', 'SOLID', 'DASH', 'DOTTED', 'BOLD', 'ITALIC', 'UPRIGHT', 'NORMAL', "HOUR", "PT", "INCH", "MM", "CM"]
__SLEEP_SLICE_IN_MILLISECONDS__ = 500
__PT_TO_TWIP__ = 20
__MM_TO_PT__ = 1/(25.4/72)
__MM10_TO_TWIP__ = 1/(2540.0/72/20) # 0.01 mm to twentieth point
__FILLCOLOR__ = 0x8000cc00
__LINEWIDTH__ = 0.5 * __PT_TO_TWIP__
__ENCODED_STRING__ = "_s_%s___"
__ENCODED_COMMENT__ = "_c_%s___"
__DECODE_STRING_REGEX__ = "_s_([0-9]+)___"
__DECODE_COMMENT_REGEX__ = "_c_([0-9]+)___"
__LINEBREAK__ = "#_@L_i_N_e@_#"
__TURTLE__ = "turtle"
__ACTUAL__ = "actual"
__BASEFONTFAMILY__ = "Linux Biolinum G"
__LineStyle_DOTTED__ = 2

class __Doc__:
    def __init__(self, doc):
        self.doc = doc
        try:
            self.drawpage = doc.DrawPage # Writer
        except:
            self.drawpage = doc.DrawPages.getByIndex(0) # Draw, Impress
        self.shapecache = {}
        self.shapecount = itertools.count()
        self.time = 0
        self.zoomvalue = 0
        self.initialize()

    def initialize(self):
        self.pen = 1
        self.pencolor = 0
        self.pensize = __LINEWIDTH__
        self.linestyle = __LineStyle_SOLID__
        self.linejoint = __ROUNDED__
        self.linecap = __Cap_NONE__
        self.oldlc = 0
        self.oldlw = 0
        self.oldls = __LineStyle_SOLID__
        self.oldlj = __ROUNDED__
        self.oldlcap = __Cap_NONE__
        self.continuous = True
        self.areacolor = __FILLCOLOR__
        self.t10y = int((__FILLCOLOR__ >> 24) / (255.0/100))
        self.hatch = None
        self.textcolor = 0
        self.fontfamily = __BASEFONTFAMILY__
        self.fontheight = 12
        self.fontweight = 100
        self.fontstyle = 0
        self.points = []

from math import pi, sin, cos, asin, sqrt, log10

from com.sun.star.awt import Point as __Point__
from com.sun.star.awt import Gradient as __Gradient__
from com.sun.star.awt.GradientStyle import LINEAR as __GradientStyle_LINEAR__
from com.sun.star.drawing import LineDash as __LineDash__
from com.sun.star.drawing import Hatch as __Hatch__
from com.sun.star.drawing import PolyPolygonBezierCoords as __Bezier__
from com.sun.star.text.TextContentAnchorType import AT_PAGE as __AT_PAGE__
from com.sun.star.text.WrapTextMode import THROUGHT as __THROUGHT__
from com.sun.star.drawing.LineCap import BUTT as __Cap_NONE__
from com.sun.star.drawing.LineCap import ROUND as __Cap_ROUND__
from com.sun.star.drawing.LineCap import SQUARE as __Cap_SQUARE__
from com.sun.star.drawing.LineJoint import NONE as __Joint_NONE__
from com.sun.star.drawing.LineJoint import BEVEL as __BEVEL__
from com.sun.star.drawing.LineJoint import MITER as __MITER__
from com.sun.star.drawing.LineJoint import ROUND as __ROUNDED__
from com.sun.star.drawing.FillStyle import NONE as __FillStyle_NONE__
from com.sun.star.drawing.FillStyle import GRADIENT as __FillStyle_GRADIENT__
from com.sun.star.drawing.LineStyle import NONE as __LineStyle_NONE__
from com.sun.star.drawing.LineStyle import SOLID as __LineStyle_SOLID__
from com.sun.star.drawing.LineStyle import DASH as __LineStyle_DASHED__
from com.sun.star.drawing.DashStyle import RECT as __DashStyle_RECT__
from com.sun.star.drawing.DashStyle import ROUND as __DashStyle_ROUND__
from com.sun.star.drawing.DashStyle import ROUNDRELATIVE as __DashStyle_ROUNDRELATIVE__
from com.sun.star.drawing.CircleKind import FULL as __FULL__
from com.sun.star.drawing.CircleKind import SECTION as __SECTION__
from com.sun.star.drawing.CircleKind import CUT as __CUT__
from com.sun.star.drawing.CircleKind import ARC as __ARC__
from com.sun.star.awt.FontSlant import NONE as __Slant_NONE__
from com.sun.star.awt.FontSlant import ITALIC as __Slant_ITALIC__
from com.sun.star.awt import Size as __Size__
from com.sun.star.awt import WindowDescriptor as __WinDesc__
from com.sun.star.awt.WindowClass import MODALTOP as __MODALTOP__
from com.sun.star.awt.VclWindowPeerAttribute import OK as __OK__ 
from com.sun.star.awt.VclWindowPeerAttribute import OK_CANCEL as __OK_CANCEL__ 
from com.sun.star.awt.VclWindowPeerAttribute import YES_NO_CANCEL as __YES_NO_CANCEL__ # OK_CANCEL, YES_NO, RETRY_CANCEL, DEF_OK, DEF_CANCEL, DEF_RETRY, DEF_YES, DEF_NO
from com.sun.star.awt.PushButtonType import OK as __Button_OK__
from com.sun.star.awt.PushButtonType import CANCEL as __Button_CANCEL__
from com.sun.star.util.MeasureUnit import APPFONT as __APPFONT__
from com.sun.star.beans import PropertyValue as __property__
from com.sun.star.lang import Locale

def __getprop__(name, value):
    p, p.Name, p.Value = __property__(), name, value
    return p

__uilocale__ = uno.getComponentContext().getValueByName("/singletons/com.sun.star.configuration.theDefaultProvider").\
    createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",\
    (__getprop__("nodepath", "/org.openoffice.Setup/L10N"),)).getByName("ooLocale") + '-' # handle missing Country of locale 'eo'

def __l12n__(lng):
    try:
        return __lng__[lng]
    except:
        try:
            __lng__[lng] = dict([[i.decode("unicode-escape").split("=")[0].strip(), i.decode("unicode-escape").split("=")[1].strip().strip("|")] for i in open(__lngpath__ + "LibreLogo_" + lng + ".properties", 'rb').readlines() if b"=" in i])
            return __lng__[lng]
        except Exception:
            __trace__()
            return None

# dot for dotted line (implemented as an array of dot-headed arrows, because PostScript dot isn't supported by Writer)
def __gendots__(n):
    return [__Point__(round(sin(360.0/n * i * pi/180.0) * 600), round(cos(360.0/n * i * pi/180) * 600)) for i in range(n)]
__bezierdot__ = __Bezier__()
__bezierdot__.Coordinates = (tuple(__gendots__(32)),)
__bezierdot__.Flags = ((0,) * 32,)

# turtle shape
__TURTLESHAPE__ = [tuple([(__Point__(-120, 130), __Point__(-245, 347), __Point__(-291, 176), ), (__Point__(0, -500), __Point__(126, -375), __Point__(0, -250), __Point__(-124, -375), ), (__Point__(295, 170), __Point__(124, 124), __Point__(250, 340), ), (__Point__(466, -204), __Point__(224, -269), __Point__(71, -180), __Point__(313, -116), ), (__Point__(-75, -175), __Point__(-292, -300), __Point__(-417, -83), ), (__Point__(250, 0), __Point__(0, -250), __Point__(-250, 0), __Point__(0, 250), )] + 
            [(i,) for i in __gendots__(32)] + # single points for wider selection
            [(__Point__(0, 0),)]), # last point for position handling
            ((__Point__(0, 0),),)] # hidden turtle (single point to draw at the left border of the page area)

def __getdocument__():
    global __docs__, _
    doc = XSCRIPTCONTEXT.getDocument()
    try:
        _ = __docs__[doc.RuntimeUID]
    except:
        _ = __Doc__(doc)
        __docs__[doc.RuntimeUID] = _

# input function, result: input string or 0
def Input(s):
    global __halt__
    try:
        ctx = uno.getComponentContext()
        smgr = ctx.ServiceManager
        text = ""

        # dialog
        d = smgr.createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", ctx)
        ps = _.doc.CurrentController.Frame.ContainerWindow.getPosSize()
        lo = _.doc.CurrentController.Frame.ContainerWindow.convertSizeToLogic(__Size__(ps.Width, ps.Height), __APPFONT__)
        d.PositionX, d.PositionY, d.Width, d.Height = lo.Width/2 - 75, lo.Height/2 - 25, 150, 50

        # label
        l = d.createInstance("com.sun.star.awt.UnoControlFixedTextModel" )

        if type(s) == list:
            text = s[1]
            s = s[0]
        l.PositionX, l.PositionY, l.Width, l.Height, l.Name, l.TabIndex, l.Label = 5, 4, 140, 14, "l1", 2, s

        # textbox or combobox
        e = d.createInstance("com.sun.star.awt.UnoControlEditModel")
        e.PositionX, e.PositionY, e.Width, e.Height, e.Name, e.TabIndex = 5, 14, 140, 12, "e1", 0

        # buttons
        b = d.createInstance( "com.sun.star.awt.UnoControlButtonModel" )
        b.PositionX, b.PositionY, b.Width, b.Height, b.Name, b.TabIndex, b.PushButtonType, b.DefaultButton =  55, 32, 45, 14, "b1", 1, __Button_OK__, True
        b2 = d.createInstance( "com.sun.star.awt.UnoControlButtonModel" )
        b2.PositionX, b2.PositionY, b2.Width, b2.Height, b2.Name, b2.TabIndex, b2.PushButtonType = 100, 32, 45, 14, "b2", 1, __Button_CANCEL__

        # insert the control models into the dialog model 
        d.insertByName( "l1", l)
        d.insertByName( "b1", b)
        d.insertByName( "b2", b2) 
        d.insertByName( "e1", e) 

        # create the dialog control and set the model 
        controlContainer = smgr.createInstanceWithContext("com.sun.star.awt.UnoControlDialog", ctx)
        controlContainer.setModel(d)

        # create a peer 
        toolkit = smgr.createInstanceWithContext("com.sun.star.awt.ExtToolkit", ctx)
        controlContainer.setVisible(False)
        controlContainer.createPeer(toolkit, None)

        # execute it
        inputtext = controlContainer.execute()
        if inputtext:
            inputtext = e.Text
        else:
            __halt__ = True

        # dispose the dialog
        controlContainer.dispose()
        return inputtext
    except Exception:
        __trace__()

def __string__(s, decimal = None): # convert decimal sign, localized BOOL and SET
    if not decimal:
        decimal = _.decimal
    if decimal == ',' and type(s) == float:
        return str(s).replace(".", ",")
    if type(s) in [list, tuple, dict, set]:
        __strings__ = []
        s = re.sub("(?u)(['\"])(([^'\"]|\\['\"])*)(?<!\\\\)\\1", __encodestring__, str(s)) # XXX fix double '\'\"'
        if decimal == ',':
            s = s.replace(".", ",")
        return re.sub(__DECODE_STRING_REGEX__, __decodestring__, \
            s.replace('set', __locname__('SET')).replace('True', __locname__('TRUE')).replace('False', __locname__('FALSE')))
    if type(s) in [str, unicode]:
        return s
    elif type(s) == bool:
        return __locname__(str(s).upper())
    return str(s)

def Print(s):
    global __halt__
    s = __string__(s, _.decimal)
    if not MessageBox(_.doc.CurrentController.Frame.ContainerWindow, s[:500] + s[500:5000].replace('\n', ' '), "", "messbox", __OK_CANCEL__):
        __halt__ = True

def MessageBox(parent, message, title, msgtype = "messbox", buttons = __OK__):
    msgtypes = ("messbox", "infobox", "errorbox", "warningbox", "querybox")
    if not (msgtype in msgtypes):
        msgtype = "messbox"
    d = __WinDesc__()
    d.Type = __MODALTOP__
    d.WindowServiceName = msgtype
    d.ParentIndex = -1
    d.Parent = parent
    d.WindowAttributes = buttons
    tk = parent.getToolkit()
    msgbox = tk.createWindow(d)
    msgbox.MessageText = message
    if title:
        msgbox.CaptionText = title
    return msgbox.execute()

def Random(r):
    try:
        return r * random.random()
    except:
        return list(r)[int(random.random() * len(r))]

def to_ascii(s):
    return s.encode("unicode-escape").decode("utf-8").replace("\\u", "__u__").replace(r"\x", "__x__")

def to_unicode(s):
    return bytes(s.replace("__x__", r"\x").replace("__u__", "\\u"), "ascii").decode("unicode-escape")

def __trace__():
    if 'PYUNO_LOGLEVEL' in os.environ:
        print(traceback.format_exc())

def __locname__(name, l = -1):
    if l == -1:
        l = _.lng
    for i in __l12n__(l):
        if i == name.upper():
            return __l12n__(l)[i].split("|")[0] # return with the first localized name
    return to_unicode(name)

def __getcursor__(fulltext):
    realselection = False
    try:
        text = _.doc.getCurrentController().getViewCursor().getText().createTextCursor() # copy selection (also in frames)
        text.gotoRange(_.doc.getCurrentController().getViewCursor(), False)
        if fulltext:
            1/len(text.getString()) # exception, if zero length
        realselection = True
    except:
        text = _.doc.getText().createTextCursorByRange(_.doc.getText().getStart())
        text.gotoEnd(True)
    return text, realselection

def __translate__(arg = None):
    global _
    __getdocument__()
    selection = __getcursor__(True)[0]
    __initialize__()
    __setlang__()
    # detect language
    text = selection.getString()
    # remove comments and strings
    text = re.sub(r"[ ]*;[^\n]*", "", re.sub(r"['„“‘«»「][^\n'”“‘’«»」]*['”“‘’«»」]", "", re.sub(r"^[ \t]*[;#][^\n]*", "", text)))
    text = " ".join(set(re.findall("(?u)\w+", text)) - set(re.findall("(?u)\w*\d+\w*", text))).lower()  # only words
    ctx = uno.getComponentContext()
    guess = ctx.ServiceManager.createInstanceWithContext("com.sun.star.linguistic2.LanguageGuessing", ctx)
    guess.disableLanguages(guess.getEnabledLanguages())
    guess.enableLanguages(tuple([Locale(i, "", "") for i in __translang__.split("|")]))
    guess = guess.guessPrimaryLanguage(text, 0, len(text))
    try:
        l = {'cs': 'cs_CZ', 'el': 'el_GR', 'en': 'en_US', 'pt': 'pt_BR'}[guess.Language]
    except:
        l = guess.Language + '_' + guess.Language.upper()
    lang = __l12n__(l)
    if not lang:
        lang = __l12n__(guess.Language)
        if not lang:
            lang = __l12n__(_.lng)
            if not lang:
                lang = __l12n__("en_US")
    lq = '\'' + lang['LEFTSTRING'].replace("|", "")
    rq = '\'' + lang['RIGHTSTRING'].replace("|", "")
    __strings__ = []

    text = re.sub(r"^(([ \t]*[;#][^\n]*))", __encodecomment__, text)
    text = re.sub("(?u)([%s])([^\n%s]*)(?<!\\\\)[%s]" % (lq, rq, rq), __encodestring__, selection.getString())
    text = re.sub('(?u)(?<![0-9])(")(~?\w*)', __encodestring__, text)
    text = re.sub(r";(([^\n]*))", __encodecomment__, text)

    # translate the program to the language of the document FIXME space/tab
    exception = ['DECIMAL']
    in1 = lang['IN'].upper()
    in2 = __l12n__(_.lng)['IN'].split("|")[0].upper()
    if in1[0] == '-' and in2[0] != '-': # "for x y-in" -> "for x in y"
        exception += ['IN']
        text = re.sub(r"(?ui)\b((?:%s) +:?\w+) +([^\n]+)(?:%s) +(?=[[] |[[]\n)" % (lang['FOR'], in1), "\\1 %s \\2 " % in2, text)
        text = re.sub(r"(?ui)(:?\b\w+|[[][^[\n]*])\b(?:%s)\b" % in1, "%s \\1" % in2, text)
    elif in1[0] != '-' and in2[0] == '-': # "for x in y" -> "for x y-in"
        exception += ['IN']
        text = re.sub(r"(?ui)(?<=\n)((?:%s)\b +:?\w+) +(?:%s) +([^\n]+?) +(?=[[] |[[]\n)" % (lang['FOR'], in1), "\\1 \\2%s " % in2, text)
        text = re.sub(r"(?ui)(?<!:)\b(?:%s) +(:?\b\w+|[[][^[\n]*])\b" % in1, "\\1%s" % in2, text)
    for i in set(lang) - set(exception):
        text = re.sub(r'(?ui)(?<!:)\b(%s)\b' % lang[i], __l12n__(_.lng)[i].split("|")[0].upper(), text)
    text = re.sub(r"(?<=\d)[%s](?=\d)" % lang['DECIMAL'], __l12n__(_.lng)['DECIMAL'], text)

    # decode strings and comments
    quoted = u"(?ui)(?<=%s)(%%s)(?=%s)" % (__l12n__(_.lng)['LEFTSTRING'][0], __l12n__(_.lng)['RIGHTSTRING'][0])
    text = re.sub(__DECODE_STRING_REGEX__, __decodestring2__, text)
    for i in __STRCONST__:
        text = re.sub(quoted % lang[i], __l12n__(_.lng)[i].split("|")[0].upper(), text)
    text = re.sub(__DECODE_COMMENT_REGEX__, __decodecomment__, text)
    if _.doc.getText().compareRegionStarts(selection.getStart(), _.doc.getText().getStart()) == 0:
        pagebreak = True
        selection.setString("\n" + text.lstrip("\n"))
    else:
        pagebreak = False
        selection.setString(text)
    # convert to paragraphs
    __dispatcher__(".uno:ExecuteSearch", (__getprop__("SearchItem.SearchString", r"\n"), __getprop__("SearchItem.ReplaceString", r"\n"), \
        __getprop__("Quiet", True), __getprop__("SearchItem.Command", 3), __getprop__("SearchItem.StyleFamily", 2), \
        __getprop__("SearchItem.AlgorithmType", 1), __getprop__("SearchItem.RowDirection", 1), __getprop__("SearchItem.SearchFlags", 65536)))
    # set 2-page layout
    if pagebreak:
        selection.getStart().BreakType = 4
    __dispatcher__(".uno:ZoomPage")

class LogoProgram(threading.Thread):
    def __init__(self, code):
        self.code = code
        threading.Thread.__init__(self)

    def run(self):
        global __thread__
        try:
            exec(self.code)
            if _.origcursor[0] and _.origcursor[1]:
                __dispatcher__(".uno:Escape")
                try:
                    _.doc.CurrentController.getViewCursor().gotoRange(_.origcursor[0], False)
                except:
                    _.doc.CurrentController.getViewCursor().gotoRange(_.origcursor[0].getStart(), False)
        except Exception as e:
            try:
              TRACEPATTERN = '"<string>", line '
              message = traceback.format_exc()
              l = re.findall(TRACEPATTERN + '[0-9]+', message)
              if len(l) > 0 and not "SystemExit" in message:
                line = len(re.findall(__LINEBREAK__, ''.join(self.code.split("\n")[:int(l[-1][len(TRACEPATTERN):])]))) + 1
                caption = __l12n__(_.lng)['LIBRELOGO']
                if __prevcode__ and "\n" in __prevcode__:
                    __gotoline__(line)
                    caption = __l12n__(_.lng)['ERROR'] % line
                parent = _.doc.CurrentController.Frame.ContainerWindow
                if "maximum recursion" in message:
                    MessageBox(parent, __l12n__(_.lng)['ERR_STOP'] + " " + __l12n__(_.lng)['ERR_MAXRECURSION'] % sys.getrecursionlimit(), __l12n__(_.lng)['LIBRELOGO'])
                elif "cannot initialize memory" in message or "Couldn't instantiate" in message:
                    MessageBox(parent, __l12n__(_.lng)['ERR_STOP'] + " " + __l12n__(_.lng)['ERR_MEMORY'], __l12n__(_.lng)['LIBRELOGO'])
                elif "ZeroDivisionError" in message:
                    MessageBox(parent, __l12n__(_.lng)['ERR_ZERODIVISION'], caption, "errorbox")
                elif "IndexError" in message:
                    MessageBox(parent, __l12n__(_.lng)['ERR_INDEX'], caption, "errorbox")
                elif "KeyError" in message:
                    MessageBox(parent, __l12n__(_.lng)['ERR_KEY'] % eval(re.search("KeyError: ([^\n]*)", message).group(1)), caption, "errorbox")
                elif "NameError" in message:
                    if "__repeat__" in message:
                        MessageBox(parent, __l12n__(_.lng)['ERR_ARGUMENTS'] % (__locname__('REPEAT'), 1, 0), caption, "errorbox")
                    else:
                        MessageBox(parent, __l12n__(_.lng)['ERR_NAME'] % \
                            to_unicode(re.search("(?<=name ')[\w_]*(?=')", message).group(0)), caption, "errorbox")
                elif "TypeError" in message and "argument" in message and "given" in message:
                    r = re.search("([\w_]*)[(][)][^\n]* (\w+) arguments? [(](\d+)", message) # XXX later: handle 'no arguments' + plural
                    MessageBox(parent, __l12n__(_.lng)['ERR_ARGUMENTS'] % (__locname__(r.group(1)), r.group(2), r.group(3)), caption, "errorbox")
                else:
                    origline = __compiled__.split("\n")[line-1]
                    if not "com.sun.star" in message and not "__repeat__" in message and not "*)" in message and ("[" in origline or "]" in origline):
                        MessageBox(parent, __l12n__(_.lng)['ERR_BLOCK'], caption, "errorbox")
                    else:
                        MessageBox(parent, __l12n__(_.lng)['ERROR'] %line, __l12n__(_.lng)['LIBRELOGO'], "errorbox")
              __trace__()
            except:
              pass
        with __lock__:
            __thread__ = None


def __encodestring__(m):
    __strings__.append(re.sub("\\[^\\]", "", m.group(2)))
    return __ENCODED_STRING__ % (len(__strings__) - 1)

def __encodecomment__(m):
    __strings__.append(re.sub("\\[^\\]", "", m.group(2)))
    return __ENCODED_COMMENT__ % (len(__strings__) - 1)

def __decodestring__(m):
    return "u'%s'" % __strings__[int(m.group(1))]

def __decodestring2__(m):
    return __l12n__(_.lng)['LEFTSTRING'][0] + __strings__[int(m.group(1))] + __l12n__(_.lng)['RIGHTSTRING'][0]

def __decodecomment__(m):
    return ";" + __strings__[int(m.group(1))]

def __initialize__():
    global __halt__, __thread__
    __getdocument__()
    _.zoomvalue = _.doc.CurrentController.getViewSettings().ZoomValue
    shape = __getshape__(__TURTLE__)
    if not shape:
        shape = _.doc.createInstance( "com.sun.star.drawing.PolyPolygonShape" )
        shape.AnchorType = __AT_PAGE__
        shape.TextWrap = __THROUGHT__
        shape.Opaque = True
        _.drawpage.add(shape) 
        shape.PolyPolygon = __TURTLESHAPE__[0]
        _.shapecache[__TURTLE__] = shape
        shape.Name = __TURTLE__
        _.initialize()
        turtlehome()
        _.doc.CurrentController.select(shape)
        shape.FillColor, transparence = __splitcolor__(_.areacolor, shape)
        shape.LineColor, shape.LineTransparence = __splitcolor__(_.pencolor)
    elif shape.Visible:
        if shape.FillStyle == __FillStyle_NONE__:
            _.areacolor = 0xffffffff
        else:
            _.areacolor = shape.FillColor + (int(255.0 * shape.FillTransparence/100) << 24)
        if shape.LineWidth != round((1 + _.pen * 2) * __PT_TO_TWIP__ / __MM10_TO_TWIP__) and shape.LineWidth != round(__LINEWIDTH__ / __MM10_TO_TWIP__):
            _.pensize = shape.LineWidth * __MM10_TO_TWIP__
        if shape.LineStyle == __LineStyle_NONE__: # - none -
            __pen__(0)
        else:
            if shape.LineStyle == __LineStyle_SOLID__:
                __pen__(1)
            _.pencolor = shape.LineColor + (int(255.0 * shape.LineTransparence/100) << 24)
    shape.LineJoint = __ROUNDED__
    shape.Shadow = True
    shape.FillColor, transparence = __splitcolor__(_.areacolor, shape)
    shape.FillTransparence = min(95, transparence)
    shape.ShadowColor, shape.ShadowTransparence, shape.ShadowXDistance, shape.ShadowYDistance = (0, 20, 0, 0)
    shape.LineWidth = min(_.pensize, (1 + _.pen * 2) * __PT_TO_TWIP__) / __MM10_TO_TWIP__
    shape.SizeProtect = True

def pagesize(n = -1):
    if n == -1:
        ps = _.doc.CurrentController.getViewCursor().PageStyleName
        page = _.doc.StyleFamilies.getByName("PageStyles").getByName(ps)
        return [page.Width * __MM10_TO_TWIP__ / __PT_TO_TWIP__, page.Height * __MM10_TO_TWIP__ / __PT_TO_TWIP__]
    return None

def turtlehome():
    turtle = __getshape__(__TURTLE__)
    if turtle:
        ps = _.doc.CurrentController.getViewCursor().PageStyleName
        page = _.doc.StyleFamilies.getByName("PageStyles").getByName(ps)
        turtle.setPosition(__Point__((page.Width - turtle.BoundRect.Width)/2, (page.Height - turtle.BoundRect.Height)/2))
        turtle.LineStyle = __LineStyle_SOLID__
        turtle.LineJoint = __MITER__
        turtle.LineWidth = min(_.pensize, (1 + _.pen * 2) * __PT_TO_TWIP__) / __MM10_TO_TWIP__
        turtle.LineColor, none = __splitcolor__(_.pencolor)
        turtle.LineTransparence = 25
        turtle.RotateAngle = 0
        turtle.ZOrder = 1000

def __pen__(n):
    _.pen = n
    turtle = __getshape__(__TURTLE__)
    if turtle:
        if n:
            turtle.LineStyle = __LineStyle_SOLID__
            turtle.LineWidth = min(_.pensize, 3 * __PT_TO_TWIP__) / __MM10_TO_TWIP__
        else:
            turtle.LineStyle = __LineStyle_DASHED__
            turtle.LineDash = __LineDash__(__DashStyle_RECT__, 0, 0, 1, __PT_TO_TWIP__, __PT_TO_TWIP__)
            turtle.LineWidth = min(_.pensize, __PT_TO_TWIP__) / __MM10_TO_TWIP__


def __visible__(shape, visible = -1): # for OOo 3.2 compatibility
    try:
        if visible == -1:
            return shape.Visible
        shape.Visible = visible
    except:
        return True

def hideturtle():
    turtle = __getshape__(__TURTLE__)
    if turtle and turtle.Visible:
        z = turtle.getPosition()
        z = __Point__(z.X + turtle.BoundRect.Width / 2.0, z.Y + turtle.BoundRect.Height / 2.0)
        turtle.PolyPolygon = __TURTLESHAPE__[1]
        __visible__(turtle, False)
        turtle.LineTransparence, turtle.FillTransparence = 100, 100 # for saved files
        turtle.setPosition(z)
    __dispatcher__(".uno:Escape")

def showturtle():
    turtle = __getshape__(__TURTLE__)
    if turtle and not turtle.Visible:
        if not turtle.Parent:
            _.drawpage.add(turtle)
        z = turtle.getPosition()
        r, turtle.RotateAngle = turtle.RotateAngle, 0
        turtle.PolyPolygon, turtle.RotateAngle = __TURTLESHAPE__[0], r
        z = __Point__(z.X - turtle.BoundRect.Width / 2.0, z.Y - turtle.BoundRect.Height / 2.0) 
        turtle.setPosition(z)
        __visible__(turtle, True)
        pencolor(_.pencolor)
        fillcolor(_.areacolor)
        pensize(_.pensize/__PT_TO_TWIP__)
        _.doc.CurrentController.select(__getshape__(__TURTLE__))
    elif not turtle:
        __initialize__()

def left(arg=None):
    if __thread__:
        return None
    __initialize__()
    turtle = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
    turtle.add(__getshape__(__TURTLE__))
    _.doc.CurrentController.select(turtle)
    rotate(__TURTLE__, 1500)
    return None

def right(arg=None):
    if __thread__:
        return None
    __initialize__()
    turtle = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
    turtle.add(__getshape__(__TURTLE__))
    _.doc.CurrentController.select(turtle)
    rotate(__TURTLE__, -1500)
    return None

def goforward(arg=None):
    if __thread__:
        return None
    __initialize__()
    turtle = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
    turtle.add(__getshape__(__TURTLE__))
    _.doc.CurrentController.select(turtle)
    forward(10)
    return None

def gobackward(arg=None):
    if __thread__:
        return None
    __initialize__()
    turtle = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
    turtle.add(__getshape__(__TURTLE__))
    _.doc.CurrentController.select(turtle)
    backward(10)
    return None

def commandline(arg=None, arg2=None):
    run(arg, arg2)

def __setlang__():
        global _
        c = _.doc.CurrentController.getViewCursor()
        locs = [i for i in [c.CharLocale, c.CharLocaleAsian, c.CharLocaleComplex] if i.Language != 'zxx'] # not None language
		# FIXME-BCP47: this needs adaption to language tags, a simple split on
		# '-' and assuming second field would be country would already fail if
		# a script tag was present.
        loc = Locale(__uilocale__.split('-')[0], __uilocale__.split('-')[1], '')
        if locs and loc not in locs:
            loc = locs[0]
        _.lng = loc.Language + '_' + loc.Country
        if not __l12n__(_.lng):
            _.lng = loc.Language
            if not __l12n__(_.lng):
                _.lng = "en_US"

def run(arg=None, arg2 = -1):
    global _, __thread__, __halt__, _, __prevcode__, __prevlang__, __prevcompiledcode__
    if __thread__:
        return None
    with __lock__:
        __thread__ = 1
    try:
        __getdocument__()
        _.origcursor = [None, None]
        if arg2 == -1:
            _.origcursor, _.cursor = __getcursor__(False), __getcursor__(True)[0]
            __dispatcher__(".uno:Escape")
            c = _.doc.Text.createTextCursor() # go to the first page
            c.gotoStart(False)
            _.doc.CurrentController.getViewCursor().gotoRange(c, False)
            __initialize__()
            __setlang__()
            arg2 = _.cursor.getString()
            if len(arg2) > 20000:
                if MessageBox(_.doc.CurrentController.Frame.ContainerWindow, __l12n__(_.lng)['ERR_NOTAPROGRAM'], __l12n__(_.lng)['LIBRELOGO'], "querybox", __YES_NO_CANCEL__) != 2:
                    with __lock__:
                        __thread__ = None
                    return None
            elif len(arg2) == 0 and _.origcursor[1]:
                _.origcursor[0].setString("fontcolor 'green'\nlabel 'LIBRE'\npu\nback 30\npic [\n\tfc any\n\tcircle 40\n\tfontcolor 'black'\n\tlabel 'LOGO'\n\tleft 180\n\tfd 20\n\tpd\n\tpc any\n\tps 1\n\tfd 40\n\trepeat 20 [\n\t\tfd repcount*2\n\t\trt 90\n\t]\n]\npu pos any pd")
                __translate__()
                _.origcursor, _.cursor = __getcursor__(False), __getcursor__(True)[0]
                arg2 = _.cursor.getString()
        else:
            __initialize__()
            __setlang__()
        if __prevcode__ and __prevcode__ == arg2 and __prevlang__ == _.lng:
            __thread__ = LogoProgram(__prevcompiledcode__)
        else:
            __prevcode__ = arg2
            __prevlang__ = _.lng
            __prevcompiledcode__ = __compil__(arg2)
            __thread__ = LogoProgram(__prevcompiledcode__)
        __halt__ = False
        turtle = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
        turtle.add(__getshape__(__TURTLE__))
        _.doc.CurrentController.select(turtle)
        # set working directory for file operations
        if _.doc.hasLocation():
          name = os.chdir(unohelper.fileUrlToSystemPath(re.sub("[^/]*$", "", _.doc.getURL())))
        else:
          name = os.chdir(os.path.expanduser('~'))
        __thread__.start()
    except Exception as e:
        __thread__ = None
        __trace__()
    return None

def stop(arg=None):
    global __halt__
    with __lock__:
        __halt__ = True
    return None

def home(arg=None):
    if __thread__:
        return None
    __getdocument__()
    turtle = __getshape__(__TURTLE__)
    if turtle:
        __removeshape__(__TURTLE__)
        _.drawpage.remove(turtle)
    __initialize__()
    __dispatcher__(".uno:Escape")
    if not __halt__:
        return None
    _.pencolor = 0
    _.pensize = __LINEWIDTH__
    _.areacolor = __FILLCOLOR__
    pen = 1
    __removeshape__(__ACTUAL__)

def clearscreen(arg=None):
    if __thread__:
        return None
    __getdocument__()
    turtle = __getshape__(__TURTLE__)
    if not turtle:
        __initialize__()
        if not __halt__:
            # avoid unintentional image deletion in large documents
            if len(__getcursor__(True)[0].getString()) < 5000:
                __cs__(False)
            return
    __cs__(False)
    __dispatcher__(".uno:Escape")

def __checkhalt__():
    global __thread__, __halt__
    if __halt__:
        with __lock__:
            __thread__ = None
        sys.exit()

def __cs__(select = True):
    turtle = __getshape__(__TURTLE__)
    visible = False
    if turtle and turtle.Visible:
        __visible__(turtle, False)
        visible = True
    if _.doc.CurrentController.select(_.drawpage) and \
        _.doc.CurrentController.getSelection().ImplementationName == "com.sun.star.drawing.SvxShapeCollection":
            __dispatcher__(".uno:Delete")
    if turtle and visible:
        __visible__(turtle, True)
        if select:
            _.doc.CurrentController.select(_.drawpage)

def __dispatcher__(s, properties = (), doc = 0):
    ctx = XSCRIPTCONTEXT.getComponentContext()
    d = ctx.ServiceManager.createInstanceWithContext("com.sun.star.frame.DispatchHelper", ctx)
    if doc != 0:
      d.executeDispatch(doc.CurrentController.Frame, s, "", 0, properties)
    else:
      d.executeDispatch(_.doc.CurrentController.Frame, s, "", 0, properties)

def __getshape__(shapename):
    try:
        if _.shapecache[shapename].Parent:
            return _.shapecache[shapename]
        _.shapecache.pop(shapename)
    except:
        pass
    return None

def __angle__(deg):
    if deg == u'any':
        return random.random() * 36000
    return deg * 100

def turnleft(deg):
    rotate(__TURTLE__, __angle__(deg))

def turnright(deg):
    rotate(__TURTLE__, -__angle__(deg))

def heading(deg = -1, go = False):
    turtle = __getshape__(__TURTLE__)
    if deg == -1:
        return -turtle.RotateAngle / 100 + 360
    else:
        if deg == u'any':
            turtle.RotateAngle = random.random() * 36000
        elif type(deg) == list:
            pos = turtle.getPosition()
            px, py = pos.X + turtle.BoundRect.Width / 2.0, pos.Y + turtle.BoundRect.Height / 2.0
            dx = px * __MM10_TO_TWIP__ - deg[0] * __PT_TO_TWIP__
            dy = deg[1] * __PT_TO_TWIP__ - py * __MM10_TO_TWIP__
            n = sqrt(dx**2 + dy**2)
            if dy > 0 and n > 0:
                turtle.RotateAngle = a = -(180 + asin(dx / n) / (pi/180)) * 100 + 72000 # +720 for max(angle, preciseAngle) of __go__()
            elif n > 0:
                turtle.RotateAngle = a = asin(dx / n) / (pi/180) * 100 + 72000
            if go and n > 0:
                __go__(__TURTLE__, -n, False, a)
        else:
            turtle.RotateAngle = -deg * 100

def rotate(shapename, deg):
    shape = __getshape__(shapename)
    if shape:
        shape.RotateAngle = shape.RotateAngle + deg

def forward(n):
    if type(n) == list:
        pos = position()
        angle = heading()
        dx = n[1] * sin((pi/180) * angle) + n[0] * sin((pi/180)*(angle + 90))
        dy = n[1] * cos((pi/180) * angle) + n[0] * cos((pi/180)*(angle + 90))
        position([pos[0] + dx, pos[1] - dy])
    elif type(n) == str:
        siz = label([1, 1, n])
        shape = __getshape__(__ACTUAL__)
        pos = position()
        angle = heading()
        w, h = siz.Width / (__PT_TO_TWIP__ / __MM10_TO_TWIP__), siz.Height / (__PT_TO_TWIP__ / __MM10_TO_TWIP__)
        dx = 0 * sin((pi/180) * (angle)) + w * sin((pi/180)*(angle + 90))
        dy = 0 * cos((pi/180) * (angle)) + w * cos((pi/180)*(angle + 90))
        position([pos[0] + dx, pos[1] - dy])
        heading(angle)
    else:
        __go__(__TURTLE__, -n * __PT_TO_TWIP__)

def backward(n):
    if type(n) == list:
        forward([-n[0], -n[1]])
        turnright(180)
    else:
        __go__(__TURTLE__, n * __PT_TO_TWIP__)

def __dots__(n, pos, dx, dy, r = -1, q = 0): # dots for dotted polyline or circle
    f = [1, 4, 4, 4, 4][q]
    k = abs(int(1.0 * n / max(20, _.pensize) / 2.0 / f))
    dots = []
    px, py = pos.X, pos.Y
    for i in range(k + 1):
        if k > 0:
            if r != -1:
                px, py = pos.X + sin(((f-1)*(q-1)*30 + 360.0/f/k * i) * pi/180.0) * r[0], pos.Y + cos(((f-1)*(q-1)*30 + 360.0/f/k * i) * pi/180) * r[1]
            else:
                px, py = pos.X + round(i * dx/k), pos.Y + round(i * dy/k)
        dots += [(__Point__(px, py), __Point__(px + 7, py + 7))]
    return dots

def __draw__(d, count = True):
    shape = _.doc.createInstance( "com.sun.star.drawing." + d)
    shape.AnchorType = __AT_PAGE__
    shape.TextWrap = __THROUGHT__
    __visible__(shape, False)
    while __zoom__(): # temporary fix program halt with continuous zoom
        while __zoom__():
            __time__.sleep(0.2)
        __time__.sleep(0.2)
    _.drawpage.add(shape)
    if __group__ != 0:
        __group__.add(shape)
        if count:
            _.shapecache[next(_.shapecount)] = str(_.time)
    return shape

def __zoom__():
    z = _.doc.CurrentController.getViewSettings().ZoomValue
    if z != _.zoomvalue:
        _.zoomvalue = z
        return True
    return False

def __lefthang__(shape):
    global __grouplefthang__
    if __group__ != 0:
        p = shape.getPosition()
        if p.X < __grouplefthang__:
            __grouplefthang__ = p.X

def __go__(shapename, n, dot = False, preciseAngle = -1):
    turtle = __getshape__(shapename)
    turtlepos = None
    if shapename == __TURTLE__:
        try:
            turtlepos = turtle.PolyPolygon[-1][-1]
        except:
            pass
    pos = turtle.getPosition()
    dx = n * sin((pi/180)*(max(turtle.RotateAngle, preciseAngle)/100))
    dy = n * cos((pi/180)*(max(turtle.RotateAngle, preciseAngle)/100))
    turtle.setPosition(__Point__(pos.X + dx / __MM10_TO_TWIP__, pos.Y + dy / __MM10_TO_TWIP__))
    if (_.pencolor != _.oldlc or _.pensize != _.oldlw or _.linestyle != _.oldls or _.linejoint != _.oldlj or _.linecap != _.oldlcap):
        __removeshape__(__ACTUAL__)
        shape = None
    else:
        shape = __getshape__(__ACTUAL__)
    _.oldlw = _.pensize
    _.oldlc = _.pencolor
    _.oldls = _.linestyle
    _.oldlj = _.linejoint
    _.oldlcap = _.linecap
    if shape and not _.pen and not dot:
        _.continuous = False
        return
    c, c2 = __Point__(pos.X + turtle.BoundRect.Width / 2.0, pos.Y + turtle.BoundRect.Height / 2.0), __Point__(round(dx), round(dy))
    if shape and "LineShape" in shape.ShapeType:
            if _.continuous or dot:
                last = shape.PolyPolygon[-1][-1]
                if not (turtlepos and (abs(last.X - turtlepos.X) > 100 or abs(last.Y - turtlepos.Y) > 100) and 
                  (__group__ == 0 or (shape.getPosition().X > 0 and turtle.getPosition().X > 0))): # picture [ ] keeps hanging shapes
                    if dot or _.linestyle == __LineStyle_DOTTED__:
                         shape.PolyPolygon = tuple( list(shape.PolyPolygon) + __dots__(n, turtlepos, dx, dy))
                    else:
                        last.X = last.X + c2.X
                        last.Y = last.Y + c2.Y
                        shape.PolyPolygon = tuple( list(shape.PolyPolygon[:-1]) + [tuple( list(shape.PolyPolygon[-1]) + [last])])
                    __lefthang__(shape)
                    return
            elif turtlepos:
                shape.PolyPolygon = tuple( list(shape.PolyPolygon) + [(turtlepos, __Point__(turtlepos.X + c2.X, turtlepos.Y + c2.Y))])
                _.continuous = True
                __lefthang__(shape)
                return
    if not _.pen and not dot:
        return
    if _.pen and not dot:
        _.points = [] # new line drawing: forget the points
    shape = __draw__("PolyLineShape")
    shape.RotateAngle = 0
    shape.PolyPolygon = tuple([tuple([__Point__(0, 0)])])
    shape.setPosition(c)
    last = shape.PolyPolygon[-1][-1]
    last2 = __Point__(last.X + c2.X, last.Y + c2.Y)
    shape.LineStyle, shape.LineDash = __linestyle__(_.linestyle)
    shape.LineJoint = _.linejoint
    shape.LineCap = _.linecap
    if dot or _.linestyle == __LineStyle_DOTTED__:
        shape.PolyPolygon = tuple( list(shape.PolyPolygon) + __dots__(n, last, c2.X, c2.Y))
        shape.LineStart = __bezierdot__
        shape.LineStartCenter = True
        shape.LineStartWidth = max(20, _.pensize) / __MM10_TO_TWIP__
        shape.LineWidth = 0
    else:
        shape.PolyPolygon = tuple([tuple( list(shape.PolyPolygon[-1]) + [last2])])
        shape.LineWidth = _.pensize / __MM10_TO_TWIP__
    shape.LineColor, shape.LineTransparence = __splitcolor__(_.pencolor)
    if shape.LineTransparence == 100:
        shape.LineStyle = 0
    __visible__(shape, True)
    shape.Name = __ACTUAL__
    _.shapecache[__ACTUAL__] = shape
    _.oldlw = _.pensize
    _.oldlc = _.pencolor
    _.oldls = _.linestyle
    _.oldlj = _.linejoint
    _.oldlcap = _.linecap
    _.continuous = True
    __lefthang__(shape)

def __fillit__(filled = True):
    oldshape = __getshape__(__ACTUAL__)
    if (oldshape and oldshape.LineStartCenter) or _.points:
        if oldshape:
            __removeshape__(__ACTUAL__)  # FIXME close dotted polyline
        if _.points:
            p = position()
            h = heading()
            for i in _.points:
                position(i)
                __pen__(1)
                __checkhalt__()
            _.points = []
            __fillit__(filled)
            __pen__(0)
            position(p)
            heading(h)
        return
    if oldshape and "LineShape" in oldshape.ShapeType:
        shape = __draw__("PolyPolygonShape", False)
        shape.PolyPolygon = oldshape.PolyPolygon
        shape.setPosition(oldshape.getPosition())
        shape.LineStyle, shape.LineDash = __linestyle__(_.linestyle)
        shape.LineJoint = _.linejoint
        shape.LineCap = _.linecap
        shape.LineWidth = _.pensize / __MM10_TO_TWIP__
        shape.LineColor, shape.LineTransparence = __splitcolor__(_.pencolor)
        shape.FillColor, shape.FillTransparence = __splitcolor__(_.areacolor, shape)
        if _.hatch:
            shape.FillBackground = True if shape.FillTransparence != 100 else False
            shape.FillHatch = _.hatch
            shape.FillStyle = 3
        elif type(_.areacolor) != tuple:
            shape.FillStyle = int(filled)
        if shape.LineTransparence == 100:
            shape.LineStyle = 0
        if shape.FillTransparence == 100:
            shape.FillTransparence = 0 # for hatching and better modifications on UI 
            if not _.hatch:
                shape.FillStyle = 0
        shape.setString(oldshape.getString())
        oldshape.Name = ""
        shape.Name = __ACTUAL__
        _.shapecache[__ACTUAL__] = shape
        if __group__ != 0:
            __group__.remove(oldshape)
        __visible__(shape, True)
        _.drawpage.remove(oldshape)
    elif oldshape and "PolyPolygon" in oldshape.ShapeType:
        oldshape.LineStyle = int(_.pen)
        oldshape.LineJoint = _.linejoint
        oldshape.LineCap = _.linecap
        if _.hatch:
            oldshape.FillBackground = True
            oldshape.FillHatch = _.hatch
            oldshape.FillStyle = 3
        else:
            oldshape.FillStyle = int(filled)
        oldshape.LineWidth = _.pensize / __MM10_TO_TWIP__
        oldshape.LineColor, oldshape.LineTransparence = __splitcolor__(_.pencolor)
        oldshape.FillColor, oldshape.FillTransparence = __splitcolor__(_.areacolor, oldshape)

def point():
    oldpen, _.pen = _.pen, 1
    oldstyle, _.linestyle = _.linestyle, __LineStyle_DOTTED__
    __go__(__TURTLE__, 0, True)
    _.pen, _.linestyle = oldpen, oldstyle
    _.points.append(position())

def __boxshape__(shapetype, l):
    turtle = __getshape__(__TURTLE__)
    shape = __draw__(shapetype + "Shape")
    pos = turtle.getPosition()
    pos.X = pos.X - (l[0] * __PT_TO_TWIP__ / __MM10_TO_TWIP__ / 2) + turtle.BoundRect.Width / 2.0
    pos.Y = pos.Y - (l[1] * __PT_TO_TWIP__ / __MM10_TO_TWIP__ / 2) + turtle.BoundRect.Height / 2.0
    shape.setPosition(pos)
    shape.setSize(__Size__(l[0] * __PT_TO_TWIP__ / __MM10_TO_TWIP__, l[1] * __PT_TO_TWIP__ / __MM10_TO_TWIP__))
    shape.LineStyle, shape.LineDash = __linestyle__(_.linestyle)
    shape.LineWidth = _.pensize / __MM10_TO_TWIP__
    shape.LineJoint = _.linejoint
    shape.LineCap = _.linecap
    shape.LineColor, shape.LineTransparence = __splitcolor__(_.pencolor)
    shape.FillColor, shape.FillTransparence = __splitcolor__(_.areacolor, shape, turtle.RotateAngle)
    if _.hatch:
        shape.FillBackground = True if shape.FillTransparence != 100 else False
        shape.FillHatch = _.hatch
        shape.FillStyle = 3
    elif type(_.areacolor) != tuple:
        shape.FillStyle = 1
    if shape.LineTransparence == 100:
        shape.LineStyle = 0
    if shape.FillTransparence == 100:
        shape.FillTransparence = 0 # for hatching and better modifications on UI 
        if not _.hatch:
            shape.FillStyle = 0
    shape.RotateAngle = turtle.RotateAngle
    if shapetype == "Rectangle" and len(l) > 2:
        shape.CornerRadius = (l[2] * __PT_TO_TWIP__) / __MM10_TO_TWIP__
    elif shapetype == "Ellipse" and len(l) > 2:
        oldBoundRect = shape.BoundRect
        try:
            shape.CircleKind = __SECTION__
            shape.CircleStartAngle = (-l[3] - 270) * 100
            shape.CircleEndAngle = (-l[2] - 270) * 100
            shape.CircleKind = [__FULL__, __SECTION__, __CUT__, __ARC__][l[4]]
        except:
            pass
        pos.X = pos.X + shape.BoundRect.X - oldBoundRect.X
        pos.Y = pos.Y + shape.BoundRect.Y - oldBoundRect.Y
        shape.setPosition(pos)
    __visible__(shape, True)
    __removeshape__(__ACTUAL__)
    _.shapecache[__ACTUAL__] = shape
    __lefthang__(shape)

def ellipse(l):
    if type(l) != type([]): # default for circle and square
        l = [l, l]
    if _.linestyle == __LineStyle_DOTTED__:
        __groupstart__()
        _.linestyle = __LineStyle_SOLID__
        pc, _.pencolor = _.pencolor, 0xff000000
        ellipse(l)
        _.pencolor, _.linestyle = pc, __LineStyle_DOTTED__
        point()
        shape = __getshape__(__ACTUAL__)
        shape.PolyPolygon = tuple(__dots__(max(l[0], l[1]) * pi * __PT_TO_TWIP__, shape.PolyPolygon[0][0], 0, 0, [i/2.0 * __PT_TO_TWIP__ for i in l]))
        turtle = __getshape__(__TURTLE__)
        shape.RotateAngle = turtle.RotateAngle
        __groupend__()
    else:
        __boxshape__("Ellipse", l)

def rectangle(l):
    if type(l) != type([]): # default for circle and square
        l = [l, l]
    if _.linestyle == __LineStyle_DOTTED__:
        __groupstart__()
        _.linestyle = __LineStyle_SOLID__
        pc, _.pencolor = _.pencolor, 0xff000000
        rectangle(l)
        _.pencolor, _.linestyle = pc, __LineStyle_DOTTED__
        point()
        shape = __getshape__(__ACTUAL__)
        if type(l) != type([]):
            l = [l, l]
        if len(l) == 2:
            l = l + [0]
        l = [i * __PT_TO_TWIP__ for i in l]
        c = shape.PolyPolygon[0][0]
        k = [min(l[0] / 2.0, l[2]), min(l[1] / 2.0, l[2])]
        p = __dots__(l[0] - 2 * k[0], __Point__(c.X - l[0]/2 + k[0], c.Y - l[1]/2), l[0] - 2 * k[0], 0)
        p = p[:-1] + __dots__(l[1] - 2 * k[1], __Point__(c.X + l[0]/2, c.Y - l[1]/2 + k[1]), 0, l[1] - 2 * k[1]) 
        p = p[:-1] + __dots__(l[0] - 2 * k[0], __Point__(c.X + l[0]/2 - k[0], c.Y + l[1]/2), -l[0] + 2 * k[0], 0) 
        p = p[:-1] + __dots__(l[1] - 2 * k[1], __Point__(c.X - l[0]/2, c.Y + l[1]/2 - k[1]), 0, -l[1] + 2 * k[1]) 
        if l[2] > 0:
               p = p + __dots__(max(k) * 2 * pi, __Point__(c.X - l[0]/2 + k[0], c.Y - l[1]/2 + k[1]), 0, 0, k, 3)[1:]
               p = p + __dots__(max(k) * 2 * pi, __Point__(c.X + l[0]/2 - k[0], c.Y - l[1]/2 + k[1]), 0, 0, k, 2)[1:]
               p = p + __dots__(max(k) * 2 * pi, __Point__(c.X + l[0]/2 - k[0], c.Y + l[1]/2 - k[1]), 0, 0, k, 1)[1:]
               p = p + __dots__(max(k) * 2 * pi, __Point__(c.X - l[0]/2 + k[0], c.Y + l[1]/2 - k[1]), 0, 0, k, 4)[1:]
        shape.PolyPolygon = tuple(p)
        turtle = __getshape__(__TURTLE__)
        shape.RotateAngle = turtle.RotateAngle
        __groupend__()
    else:
        __boxshape__("Rectangle", l)

def label(st):
    if type(st) != type([]):
        st = [0, 0, st]
    # get text size 
    shape = _.doc.createInstance( "com.sun.star.drawing.TextShape")
    shape.TextAutoGrowWidth = True
    shape.Visible = False
    actual = __getshape__(__ACTUAL__)
    _.drawpage.add(shape)
    text(shape, st[2])
    z = shape.getSize()
    # show text using RectangleShape (for correct SVG export)
    ac, pc =  _.areacolor, _.pencolor
    _.areacolor, _.pencolor = 0xff000000, 0xff000000 # invisible
    rectangle([z.Width / (__PT_TO_TWIP__ / __MM10_TO_TWIP__), z.Height / (__PT_TO_TWIP__ / __MM10_TO_TWIP__)])
    _.drawpage.remove(shape)
    _.pencolor, _.areacolor = pc, ac
    lab = __getshape__(__ACTUAL__) 
    text(lab, st[2])
    if st[0] != 0 or st[1] != 0:
        pos = position()
        angle = heading()
        n = [st[0] * z.Width/2, st[1] * z.Height/2]
        dx = n[1] * sin((pi/180) * angle) + n[0] * sin((pi/180)*(angle + 90))
        dy = n[1] * cos((pi/180) * angle) + n[0] * cos((pi/180)*(angle + 90)) 
        lab.setPosition(__Point__(round(pos[0] * __PT_TO_TWIP__ / __MM10_TO_TWIP__ + dx - lab.BoundRect.Width/2), round(pos[1] * __PT_TO_TWIP__ / __MM10_TO_TWIP__ - dy - lab.BoundRect.Height/2)))
    _.shapecache[__ACTUAL__] = actual
    return z

def text(shape, st):
    if shape:
        shape.setString(__string__(st, _.decimal))
        c = shape.createTextCursor()
        c.gotoStart(False)
        c.gotoEnd(True)
        c.CharColor, none = __splitcolor__(_.textcolor)
        c.CharHeight = _.fontheight
        c.CharWeight = __fontweight__(_.fontweight)
        c.CharPosture = __fontstyle__(_.fontstyle)
        c.CharFontName = _.fontfamily

def sleep(t):
    _.time = _.time + t
    __removeshape__(__ACTUAL__)
    for i in range(int(t/__SLEEP_SLICE_IN_MILLISECONDS__)):
        __checkhalt__()
        __time__.sleep(0.5)
    __checkhalt__()
    __time__.sleep(t%__SLEEP_SLICE_IN_MILLISECONDS__/1000.0)

def __removeshape__(shapename):
    try:
        _.shapecache.pop(shapename).Name = ""
    except:
        pass

def __fontweight__(w):
    if type(w) == int:
        return w
    elif re.match(__l12n__(_.lng)['BOLD'], w, flags = re.I):
        return 150
    elif re.match(__l12n__(_.lng)['NORMAL'], w, flags = re.I):
        return 100
    return 100

def __fontstyle__(w):
    if type(w) == int:
        return w
    elif re.match(__l12n__(_.lng)['ITALIC'], w, flags = re.I):
        return __Slant_ITALIC__
    elif re.match(__l12n__(_.lng)['UPRIGHT'], w, flags = re.I):
        return __Slant_NONE__
    return __Slant_NONE__

def __color__(c):
    if type(c) in [int, float, long]:
        return c
    if type(c) == unicode:
        if c == u'any':
            rc, rv, rgray = __NORMCOLORS__[int(random.random()*7)], random.random(), random.random() ** 0.5
            ratio = 1.0*abs(rc[2])/(abs(rc[2]) + abs(rc[4]))
            newcol = list(rc[0])
            if rv < ratio:
                newcol[rc[1]] += rc[2] * rv/ratio
            else:
                newcol[rc[3]] += rc[4] * (rv - ratio)/(1 - ratio)
            # random grayness
            rdark = 1 - 2**4 * (random.random()-0.5)**4
            for i in range(0, 3):
                newcol[i] = 255 * (rgray + (newcol[i]/255.0 - rgray) * rdark)
            return __color__(newcol)
        if c[0:1] == '~':
            c = __componentcolor__(__colors__[_.lng][c[1:].lower()])
            for i in range(3):
                c[i] = max(min(c[i] + int(random.random() * 64) - 32, 255), 0)
            return __color__(c)
        return __colors__[_.lng][c.lower()]
    if type(c) == list:
        if len(c) == 1: # color index
            return __COLORS__[int(c[0])][1]
        elif len(c) == 3: # RGB
            return (int(c[0])%256 << 16) + (int(c[1])%256 << 8) + int(c[2])%256
        elif len(c) == 2 or len(c) > 4: # gradient
           return (__color__(c[0]), __color__(c[1])) + tuple(c[2:])
    return (int(c[3])%256 << 24) + (int(c[0])%256 << 16) + (int(c[1])%256 << 8) + int(c[2])%256 # RGB + alpha

def __linestyle__(s):
    if _.pen == 0:
        return 0, __LineDash__()
    if _.linestyle == __LineStyle_DASHED__:
        return _.linestyle, __LineDash__(__DashStyle_RECT__, 0, 0, 1, 100, 100)
    elif _.linestyle == __LineStyle_DOTTED__:
        return __LineStyle_DASHED__, __LineDash__(__DashStyle_RECT__, 1, 1, 0, 0, 100000)
    elif type(s) == list:
        return __LineStyle_DASHED__, __LineDash__((s[5:6] or [0])[0], s[0], s[1] * __PT_TO_TWIP__, s[2], s[3] * __PT_TO_TWIP__, s[4] * __PT_TO_TWIP__)
    return s, __LineDash__()

def fillstyle(s):
    if type(s) == list:
        color, null = __splitcolor__(__color__(s[1]))
        _.hatch = __Hatch__(s[0] - 1, color, s[2] * __PT_TO_TWIP__, s[3] * 10)
    elif s == 0:
        _.hatch = None
    elif s <= 10: # using hatching styles of Writer
        fillstyle([[1, 0, 5, 0], [1, 0, 5, 45], [1, 0, 5, -45], [1, 0, 5, 90], [2, [127, 0, 0], 5, 45], [2, [127, 0, 0], 5, 0], [2, [0, 0, 127], 5, 45], [2, [0, 0, 127], 5, 0], [3, [0, 0, 127], 5, 0], [1, 0, 25, 45]][s-1])

def __splitcolor__(c, shape = None, angle = None):
    if shape and (type(c) == tuple or type(_.t10y) == list):
        angle = heading() if angle == None else -angle / 100 + 360
        if type(c) == tuple:
            shape.FillStyle = __FillStyle_GRADIENT__
            # gradient color: [color1, color2, style, angle(must be positive for I/O), border, x_percent, y_percent, color1_intensity_percent, color2_intensity_percent]
            d, d[0:len(c)], c = [0, 0, __GradientStyle_LINEAR__, 0, 0, 0, 0, 100, 100], c, c[0]
            shape.FillGradient = __Gradient__(d[2], d[0], d[1], (-angle + d[3]) * 10 % 3600, d[4], d[5], d[6], d[7], d[8], 0)
        if type(_.t10y) == list: # transparency gradient: [begin_percent, end_percent, style, angle, border, x_percent, y_percent]
            table = _.doc.createInstance("com.sun.star.drawing.TransparencyGradientTable")
            if not table.hasByName(str(_.t10y) + str(angle)):
                t, t[0:len(_.t10y)] = [100, __GradientStyle_LINEAR__, 0, 0, 0, 0, 0], _.t10y
                table.insertByName(str(_.t10y) + str(angle), __Gradient__(t[2], t[0] * 0xffffff / 100.0, t[1] * 0xffffff / 100.0, (-angle + t[3]) * 10 % 3600, t[4], t[5], t[6], 100, 100, 0))
            shape.FillTransparenceGradientName = str(_.t10y) + str(angle)
            c = 0 if type(c) == tuple else c & 0xffffff
        else:
            shape.FillStyle = __FillStyle_GRADIENT__
            c = int(_.t10y * 255.0/100) << 24
    """Split color constants to RGB (3-byte) + transparency (%)"""
    return int(c) & 0xffffff, (int(c) >> 24) / (255.0/100)

def __componentcolor__(c):
    a = [ (c & 0xff0000) >> 16, (c & 0xff00) >> 8, c & 0xff ]
    if c > 2**24:
        a.append((c & 0xff000000) >> 24)
    return a

def pencolor(n = -1):
    if n != -1:
        _.pencolor = __color__(n)
        turtle = __getshape__(__TURTLE__)
        if turtle and __visible__(turtle):
            turtle.LineColor, turtle.LineTransparence = __splitcolor__(_.pencolor)
    else:
        return __componentcolor__(_.pencolor)

def pensize(n = -1):
    if n != -1:
        if n == 'any':
            _.pensize = random.random() * 10 * __PT_TO_TWIP__
        else:
            _.pensize = n * __PT_TO_TWIP__
        turtle = __getshape__(__TURTLE__)
        if turtle and __visible__(turtle):
            turtle.LineWidth = min(_.pensize, (1 + _.pen * 2) * __PT_TO_TWIP__) / __MM10_TO_TWIP__
    return _.pensize / __PT_TO_TWIP__

def penstyle(n = -1):
    if n == -1:
        try:
            return __locname__(_.linestyle.value)
        except:
            return __locname__('DOTTED')
    if type(n) == list and len(n) >= 5:
        _.linestyle = n
    elif re.match(__l12n__(_.lng)['SOLID'], n, flags = re.I):
        _.linestyle = __LineStyle_SOLID__
    elif re.match(__l12n__(_.lng)['DASH'], n, flags = re.I):
        _.linestyle = __LineStyle_DASHED__
    elif re.match(__l12n__(_.lng)['DOTTED'], n, flags = re.I):
        _.linestyle = __LineStyle_DOTTED__

def penjoint(n = -1):
    if n == -1:
        return __locname__(_.linejoint.value)
    if re.match(__l12n__(_.lng)['NONE'], n, flags = re.I):
        _.linejoint = __Joint_NONE__
    elif re.match(__l12n__(_.lng)['BEVEL'], n, flags = re.I):
        _.linejoint = __BEVEL__
    elif re.match(__l12n__(_.lng)['MITER'], n, flags = re.I):
        _.linejoint = __MITER__
    elif re.match(__l12n__(_.lng)['ROUNDED'], n, flags = re.I):
        _.linejoint = __ROUNDED__

def pencap(n = -1):
    if n == -1:
        return __locname__(_.linecap.value.replace('BUTT', 'NONE'))
    if re.match(__l12n__(_.lng)['NONE'], n, flags = re.I):
        _.linecap = __Cap_NONE__
    elif re.match(__l12n__(_.lng)['ROUNDED'], n, flags = re.I):
        _.linecap = __Cap_ROUND__
    elif re.match(__l12n__(_.lng)['SQUARE'], n, flags = re.I):
        _.linecap = __Cap_SQUARE__

def fillcolor(n = -1):
    if n != -1:
        _.areacolor = __color__(n)
        if type(_.areacolor) != tuple:
            _.t10y = (int(_.areacolor) >> 24) / (255.0/100)
        else:
            _.t10y = 0
        turtle = __getshape__(__TURTLE__)
        if turtle and __visible__(turtle):
            turtle.FillColor, transparence = __splitcolor__(_.areacolor, turtle)
            turtle.FillTransparence = min(95, transparence)
    else:
        return __componentcolor__(_.areacolor)

def filltransparency(n = -1):
    if n != -1:
        if n == u'any':
            n = 100 * random.random()
        if type(n) != list:
            if type(_.areacolor) != tuple:
                fillcolor((_.areacolor & 0xffffff) + (int(n * (255.0/100)) << 24))
            else:
                _.t10y = n
        else:
            _.t10y = n
    else:
        return _.t10y

def pentransparency(n = -1):
    if n != -1:
        if n == u'any':
            n = 100 * random.random()
        pencolor((_.pencolor & 0xffffff) + (int(n * (255.0/100)) << 24))
    else:
        return _.pencolor >> 24

def fontcolor(n = -1):
    if n != -1:
        _.textcolor = __color__(n)
    else:
        return __componentcolor__(_.textcolor)

def position(n = -1):
    turtle = __getshape__(__TURTLE__)
    if turtle:
        if n != -1:
            if n == 'any':
                ps = pagesize()
                heading([random.random() * ps[0], random.random() * ps[1]], True)
            else:
                heading(n, True)
        else:
            pos = turtle.getPosition()
            pos.X, pos.Y = pos.X + turtle.BoundRect.Width / 2.0, pos.Y + turtle.BoundRect.Height / 2.0
            return [ pos.X * __MM10_TO_TWIP__ / __PT_TO_TWIP__, pos.Y * __MM10_TO_TWIP__ / __PT_TO_TWIP__ ]

def __groupstart__(name = ""):
    global __group__, __grouplefthang__, __groupstack__
    __removeshape__(__ACTUAL__)
    __groupstack__.append(__group__)
    if name != "": # store pic name (for correct repcount)
      __groupstack__.append(name)
      if ".SVG" == name[-4:].upper():
          _.time = 0
          _.shapecount = itertools.count()
    __groupstack__.append(__grouplefthang__)
    __group__ = uno.getComponentContext().ServiceManager.createInstance('com.sun.star.drawing.ShapeCollection')
    __grouplefthang__ = 0

def create_svg_animation(m):
    global _
    id = int(m.group(1))
    if id - 3 in _.shapecache:
        t = _.shapecache[id-3]
        opacity = "100" if t == "0" else "0"
        name = "" if id != 3 else "id=\"first\""
        start = "%sms;last.end+%sms" % (t, t) if id == 3 else "first.end+%dms" % (int(t) - int(_.shapecache[0]))
        return '<g id="id%s" opacity="0"><animate %s attributeName="opacity" from="100" to="100" begin="%s" dur="1ms" fill="freeze"/><animate attributeName="opacity" from="100" to="%s" begin="last.end" dur="1ms" fill="freeze"/>' % (m.group(1), name, start, opacity)
    return m.group()

def create_valid_svg_file(filename):
    with open(filename, "r") as f:
        s = f.read()
    s = re.sub('(?s)(<g\\sid="[^"]*)\(([^"]*)\)', '\\1\\2', s) # bad "(", ")" in xml:id
    s = re.sub('(?s)<g\\sooo:[^>]*>', '', s) # remove non standard attributes
    s = re.sub('(?s)<defs class="EmbeddedBulletChars">.*(?=<defs class="TextEmbeddedBitmaps")', '', s) # remove unused parts
    s = re.sub('(?s)(<path stroke-width="[^"]*"[^<]*)stroke-width="[^"]*"', '\\1', s) # double stroke-width
    s = re.sub('(?s)<svg\\s+version="1.2"', '<svg version="1.1"', s) # for W3C Validator
    if _.time > 0:
        s = re.sub('<g id="id([0-9]+)">', create_svg_animation, s)
        m = re.match('(?s)(.*<animate[^>]*first[.]end.([0-9]+)[^>]* dur=")1ms"', s)
        lasttime = _.time - int(m.group(2)) - int(_.shapecache[0]) + 1
        if lasttime > 1:
            s = re.sub('(?s)(.*<animate[^>]*first[.]end.([0-9]+)[^>]* dur=")1ms"', m.group(1) + str(lasttime) + 'ms" id="last"',  s)
    with open(filename, 'w') as f:
        f.write(s)

def __groupend__(name = ""):
    global __group__, __grouplefthang__, __groupstack__, __halt__
    g = 0
    if __group__.getCount() > 1:
        if __grouplefthang__ < 0:
            for i in range(__group__.Count):
                s = __group__.getByIndex(i)
                p = s.getPosition()
                p.X = p.X + -__grouplefthang__
                s.setPosition(p)
            g = _.drawpage.group(__group__)
            p = g.getPosition()
            p.X = p.X + __grouplefthang__
            g.setPosition(p)
        else:
            g = _.drawpage.group(__group__)
        g.TextWrap = __THROUGHT__
    elif __group__.getCount() == 1:
        g = __group__.getByIndex(0)
    __grouplefthang__ = min(__groupstack__.pop(), __grouplefthang__)
    if name != "":
      name = __groupstack__.pop()
    if name and ".SVG" == name[-4:].upper() and g != 0:
      _.doc.CurrentController.select(g)
      __dispatcher__(".uno:Copy")
      ctx = XSCRIPTCONTEXT.getComponentContext()
      d = ctx.ServiceManager.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)
      draw = d.loadComponentFromURL("private:factory/sdraw", "_blank", 0, ())
      drawpage = draw.getDrawPages().getByIndex(0)
      while XSCRIPTCONTEXT.getDocument() != draw:
        if XSCRIPTCONTEXT.getDocument() not in [draw, _.doc, None]:
          __halt__ = True
          return
        __time__.sleep(0.1)
      __dispatcher__(".uno:Paste", (), draw)
      __dispatcher__(".uno:FormatGroup", (), draw)
      pic = drawpage.getByIndex(0)
      pic.setPosition(__Point__((g.BoundRect.Width - g.Size.Width)//2, (g.BoundRect.Height - g.Size.Height)//2))
      drawpage.Height, drawpage.Width = g.BoundRect.Height, g.BoundRect.Width
      if not os.path.isabs(name):
        name = os.getcwd() + os.path.sep + name
      __dispatcher__(".uno:ExportTo", (__getprop__("URL", unohelper.systemPathToFileUrl(name)), __getprop__("FilterName", "draw_svg_Export")), draw)
      draw.close(True)
      while XSCRIPTCONTEXT.getDocument() != _.doc:
        if XSCRIPTCONTEXT.getDocument() not in [draw, _.doc, None]:
          __halt__ = True
          return
        __time__.sleep(0.1)
      create_valid_svg_file(name)
    __group__ = __groupstack__.pop()
    if __group__ != 0 and g != 0:
        __group__.add(g)
    __removeshape__(__ACTUAL__)

def __int__(x): # handle eg. int("10cm")
    if type(x) == str or type(x) == unicode:
        x = __float__(x)
    return int(x)

def __float__(x): # handle eg. float("10,5cm")
    if type(x) == str or type(x) == unicode:
        for i in __comp__[_.lng]:
            x = re.sub(u"(?iu)" + i[0], i[1], x)
        x = eval(x)
    return float(x)

def fontheight(n = -1):
    if n != -1:
        _.fontheight = n
    else:
        return _.fontheight

def fontweight(n = -1):
    if n != -1:
        _.fontweight = n
    else:
        return _.fontweight

def fontfamily(s = -1):
    if s != -1:
        _.fontfamily = s
    else:
        return _.fontfamily

def fontstyle(n = -1):
    if n != -1:
        _.fontstyle = n
    else:
        return _.fontstyle

def __loadlang__(lang, a):
    global comp, __colors__
    __colors__[lang] = {}
    for i in __COLORS__:
        for j in a[i[0]].split("|"):
            __colors__[lang][j.lower()] = i[1]
    for i in a:
        if not i[0:3] in ["LIB", "ERR", "PT", "INC", "MM", "CM", "HOU", "DEG"] and not i in __STRCONST__: # uppercase native commands
            a[i] = a[i].upper()
    repcount = a['REPCOUNT'].split('|')[0]
    loopi = itertools.count()
    loop = lambda r: "%(i)s = 1\n%(orig)s%(j)s = %(i)s\n%(i)s += 1\n" % \
        { "i": repcount + str(next(loopi)), "j": repcount, "orig": re.sub( r"(?ui)(?<!:)\b%s\b" % repcount, repcount + str(next(loopi)-1), r.group(0)) }
    __comp__[lang] = [
    [r"(?i)(?<!:)(\b|(?=[-:]))(?:%s)\b" % "|".join([a[i].lower() for i in a if not "_" in i and i != "DECIMAL"]), lambda s: s.group().upper()], # uppercase all native commands in the source code
    [r"(?<!:)\b(?:%s) \[(?= |\n)" % a['GROUP'], "\n__groupstart__()\nfor __groupindex__ in range(2):\n[\nif __groupindex__ == 1:\n[\n__groupend__()\nbreak\n]\n"],
    [r"(?<!:)\b(?:%s) (%s[^[]*)\[(?= |\n)" % (a['GROUP'], __DECODE_STRING_REGEX__), "\n__groupstart__(\\1)\nfor __groupindex__ in range(2):\n[\nif __groupindex__ == 1:\n[\n__groupend__(\\1)\nbreak\n]\n"],
    [r"(?<!:)\b(?:%s)\b" % a['GROUP'], "\n__removeshape__(__ACTUAL__)\n"],
    [r"(\n| )][ \n]*\[(\n| )", "\n]\nelse:\n[\n"], # if/else block
    [r"(?<!\n)\[(?= |\n)", ":\n[\n"], # start block
    [r"( ]|\n]$)", "\n]\n"], # finish block
    [r"(?<!:)\b(?:%s)\b" % a['FOR'], "\nfor"],
    [r"(?<!:)\b(?:%s)\b" % a['REPEAT'], "\n__repeat__"],
    [r"(?<!:)\b(?:%s)\b" % a['BREAK'], "\nbreak"],
    [r"(?<!:)\b(?:%s)\b" % a['CONTINUE'], "\ncontinue"],
    [r"(?<!:)\b(?:%s)\b" % a['REPCOUNT'], repcount],
    [r"(?<!:)\b(?:%s)\b" % a['IF'], "\nif"],
    [r"(?<!:)\b(?:%s)\b" % a['WHILE'], "\nwhile"],
    [r"(?<!:)\b(?:%s)\b" % a['OUTPUT'], "\nreturn"],
    [r"\n(if|while|return) [^\n]*", lambda r: re.sub("(?<![=!<>])=(?!=)", "==", r.group(0))], # = -> ==, XXX x = y = 1?
    [r"(?<=\n)(for\b :?\w+) ([^\n]+)(?<=\w|]|}|\))(?=-|:)(?:%s)\b" % a['IN'], "\\1 in \\2"], # "for x y-in" -> "for x in y"
    [r"(:?\b\w+|[[][^[\n]*])\b(?:%s)\b" % a['IN'], "in \\1"], # "x y-in" -> "x in y"
    [r"(?<!:)\b(?:%s)\b" % a['IN'], "in"],
    [r"(?<!:)\b(?:%s)\b[ \t]+(:?\w+)\b(?! in\b)" % a['FOR'], "\nfor \\1 in"],
    [r"(?<=\n)__repeat__ :\n", "while True:\n"], # infinite loop
    [r"(?<=\n)(for|while) (?!__groupindex__)[^\n]*:\n\[\n", loop], # loop variables for repcount (not groupindex loop)
    [r"(?<=\n)__repeat__([^\n]*\w[^\n]*):(?=\n)", "for %s in range(1, 1+int(\\1)):" % repcount], # repeat block
    [r"(?<=\d)[%s](?=\d)" % a['DECIMAL'], "."], # decimal sign
    [r"(?<!/)/(?!/)", "*1.0/"], # fix division: /1 -> /1.0, but not with //
    [r"\b([0-9]+([,.][0-9]+)?)(%s)\b" % a['HOUR'], lambda r: str(float(r.group(1).replace(",", "."))*30)], # 12h = 12*30°
    [r"(?<=\d)(%s)" % a['DEG'], ""], # 1° -> 1
    [r"(?<!:)\b(?:__def__)[ \t]+(\w+)\b[ \t]*([:]?\w[^\n]*)", "\ndef \\1(\\2):\n["],
    [r"(?<!:)\b(?:__def__)\s+(\w+)", "\ndef \\1():\n["],
    [r"(?<!:)\b(?:%s)\b" % a['END'], "\n]"],
    [r"(?<!:)\b(?:%s)\b" % a['GLOBAL'], "global"],
    [r"(?<!:)\b(?:%s)\b" % a['TRUE'], "True"],
    [r"(?<!:)\b(?:%s)\b" % a['FALSE'], "False"],
    [r"(?<!:)\b(?:%s)\b" % a['NOT'], "not"],
    [r"(?<!:)\b(?:%s)\b" % a['AND'], "and"],
    [r"(?<!:)\b(?:%s)\b" % a['OR'], "or"],
    [r"(?<!:)\b(?:%s)\b" % a['INT'], "__int__"],
    [r"(?<!:)\b(?:%s)\b" % a['FLOAT'], "__float__"],
    [r"(?<!:)\b(?:%s)\b" % a['STR'], "__string__"],
    [r"(?<!:)\b(?:%s)\b" % a['COUNT'], "len"],
    [r"(?<!:)\b(?:%s)\b" % a['ROUND'], "round"],
    [r"(?<!:)\b(?:%s)\b" % a['ABS'], "abs"],
    [r"(?<!:)\b(?:%s)\b" % a['SIN'], "sin"],
    [r"(?<!:)\b(?:%s)\b" % a['COS'], "cos"],
    [r"(?<!:)\b(?:%s)\b" % a['PI'], "pi"],
    [r"(?<!:)\b(?:%s)\b" % a['SQRT'], "sqrt"],
    [r"(?<!:)\b(?:%s)\b" % a['LOG10'], "log10"],
    [r"(?<!:)\b(?:%s)\b" % a['MIN'], "min"],
    [r"(?<!:)\b(?:%s)\b" % a['MAX'], "max"],
    [r"(?<!:)\b(?:%s)\b" % a['STOP'], "\nreturn None"],
    [r"(?<!:)\b(?:%s)\b" % a['CLEARSCREEN'], "\n__cs__()"],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENCOLOR'], "\n)pencolor("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENSTYLE'], "\n)penstyle("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENJOINT'], "\n)penjoint("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENCAP'], "\n)pencap("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FILLCOLOR'], "\n)fillcolor("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FILLTRANSPARENCY'], "\n)filltransparency("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENTRANSPARENCY'], "\n)pentransparency("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FILLSTYLE'], "\n)fillstyle("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FONTCOLOR'], "\n)fontcolor("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FONTFAMILY'], "\n)fontfamily("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FONTHEIGHT'], "\n)fontheight("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FONTWEIGHT'], "\n)fontweight("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['FONTSTYLE'], "\n)fontstyle("],
    [r"(?<!:)\b(?:%s)(\s+|$)" % a['PENWIDTH'], "\n)pensize("],
    [r"(?<!:)\b(?:%s)\b" % a['PENDOWN'], "\n__pen__(1)"],
    [r"(?<!:)\b(?:%s)\b" % a['PENUP'], "\n__pen__(0)"],
    [r"(?<!:)\b(?:%s)\b" % a['HIDETURTLE'], "\nhideturtle()"],
    [r"(?<!:)\b(?:%s)\b" % a['SHOWTURTLE'], "\nshowturtle()"],
    [r"(?<!:)\b(?:%s)\b\[" % a['POSITION'], "position()["],
    [r"(?<!:)\b(?:%s)\b(?!\()" % a['POSITION'], "\n)position("],
    [r"(?<!:)\b(?:%s)\b" % a['HEADING'], "\n)heading("],
    [r"(?<!:)\b(?:%s)\b" % a['PAGESIZE'], "pagesize()"],
    [r"(?<!:)\b(?:%s)\b" % a['POINT'], "\npoint()"],
    [r"(?<!:)\b(?:%s)\b" % (a['ELLIPSE'] + "|" + a['CIRCLE']), "\n)ellipse("],
    [r"(?<!:)\b(?:%s)\b" % (a['RECTANGLE'] + "|" + a['SQUARE']), "\n)rectangle("],
    [r"(?<!:)\b(?:%s)\b" % a['CLOSE'], "\n__fillit__(False)"],
    [r"(?<!:)\b(?:%s)\b" % a['FILL'], "\n__fillit__()"],
    [r"(?<!:)\b(?:%s)\b" % a['LABEL'], "\n)label("],
    [r"(?<!:)\b(?:%s)\b" % a['TEXT'], "\n)text(__getshape__(__ACTUAL__),"],
    [r"(text\([ \t]*\"[^\"\n\)]*)", "\\1\"\n"],
    [r"(?<!:)\b(?:%s)\b" % a['HOME'], "\nturtlehome()"],
    [r"(?<!:)\b(?:%s)\b" % a['SLEEP'], "\n)sleep("],
    [r"(?<!:)\b(?:%s)\b" % a['FORWARD'], "\n)forward("],
    [r"(?<!:)\b(?:%s)\b" % a['BACKWARD'], "\n)backward("],
    [r"(?<!:)\b(?:%s)\b" % a['TURNRIGHT'], "\n)turnright("],
    [r"(?<!:)\b(?:%s)\b" % a['RANDOM'], "Random"],
    [r"(?<!:)\b(?:%s)\b(?= \d)" % 'Random', "random.random()*"],
    [r"(?<!:)\b(?:%s)\b" % a['SET'], "set"],
    [r"(?<!:)\b(?:%s)\b" % a['RANGE'], "range"],
    [r"(?<!:)\b(?:%s)\b" % a['LIST'], "list"],
    [r"(?<!:)\b(?:%s)\b" % a['TUPLE'], "tuple"],
    [r"(?<!:)\b(?:%s)\b" % a['SORTED'], "sorted"],
    [r"(?<!:)\b(?:%s)\b ?\(" % a['RESEARCH'], "re.search('(?u)'+"],
    [r"(?<!:)\b(?:%s)\b ?\(" % a['RESUB'], "re.sub('(?u)'+"],
    [r"(?<!:)\b(?:%s)\b ?\(" % a['REFINDALL'], "re.findall('(?u)'+"],
    [r"(?<!:)\b(?:%s)\b" % a['ANY'], "u'any'"],
    [r"(?<!:)\b(?:%s) (\w+|[[][^\]]*])\b" % a['INPUT'], " Input(\\1)"],
    [r"(?<!:)\b(?:%s)\b" % a['PRINT'], "\n)Print("],
    [r"(?<!:)\b(?:%s)\b" % a['TURNLEFT'], "\n)turnleft("],
    [r"\b([0-9]+([,.][0-9]+)?)(%s)\b" % a['PT'], "\\1"],
    [r"\b([0-9]+([,.][0-9]+)?)(%s)(?!\w)" % a['INCH'], lambda r: str(float(r.group(1).replace(",", "."))*72)],
    [r"\b([0-9]+([,.][0-9]+)?)(%s)\b" % a['MM'], lambda r: str(float(r.group(1).replace(",", "."))*__MM_TO_PT__)],
    [r"\b([0-9]+([,.][0-9]+)?)(%s)\b" % a['CM'], lambda r: str(float(r.group(1).replace(",", "."))*__MM_TO_PT__*10)],
    [r"\b(__(?:int|float|string)__len|round|abs|sin|cos|sqrt|log10|set|list|tuple|sorted)\b ((?:\w|\d+([,.]\d+)?|0[xX][0-9a-fA-F]+|[-+*/]| )+)\)" , "\\1(\\2))" ], # fix parsing: (1 + sqrt x) -> (1 + sqrt(x))
    [r"(?<=[-*/=+,]) ?\n\)(\w+)\(", "\\1()"], # read attributes, eg. x = fillcolor
    [r"(?<=return) ?\n\)(\w+)\(", "\\1()"], # return + user function
    [r"(?<=(?:Print|label)\() ?\n\)(\w+)\(", "\\1()\n"] # Print/label + user function
    ]

def __concatenation__(r): # keep line positions with extra line breaks
    s = re.subn("~[ \t]*\n", " ", r.group(0))
    return s[0] + "\n" * s[1]

def __compil__(s):
    global _, comp, __strings__, __compiled__
    try:
        c = _.doc.CurrentController.getViewCursor()
        locs = [i for i in [c.CharLocale, c.CharLocaleAsian, c.CharLocaleComplex] if i.Language != 'zxx'] # not None language
        loc = Locale(__uilocale__.split('-')[0], __uilocale__.split('-')[1], '')
        if locs and loc not in locs:
            loc = locs[0]
        try:
            _.lng = loc.Language + '_' + loc.Country
            __loadlang__(_.lng, __l12n__(_.lng))
        except:
            __trace__()
            _.lng = loc.Language 
            __loadlang__(_.lng, __l12n__(_.lng))
    except:
        __trace__()
        _.lng = 'en_US'
        if not _.lng in __comp__:
            __loadlang__(_.lng, __l12n__(_.lng)) 

    _.decimal = __l12n__(_.lng)['DECIMAL']
    names = {}

    rmsp = re.compile(r"[ ]*([=+*/]|==|<=|>=|<>|!=|-[ ]+)[ ]*")
    chsp = re.compile(r"[ \t]+")
    chch = re.compile(r"(?u)(?<!\w):(?=\w)")
    parenfix = re.compile(r"(?ui)(\([^\(\[\]\)]+)]\)")

    # remove CR characters and split lines
    s = re.sub(r'[ \t\r]*(?=\n)', '', s)

    # remove full line comments
    s = re.sub(r"^[ \t]*[;#][^\n]*", "", s)
    s = re.sub(r"(?<=\n)[ \t]*[;#][^\n]*", "", s)

    # concatenate lines
    __compiled__ = re.sub(r'([^\n]*~[ \t]*\n)+[^\n]*', __concatenation__, s)

    # sign original line breaks
    s = re.sub("(?<=\n)", __LINEBREAK__ + "\n", __compiled__)

    # encode strings
    lq = '\'' + __l12n__(_.lng)['LEFTSTRING'].replace("|", "")
    rq = '\'' + __l12n__(_.lng)['RIGHTSTRING'].replace("|", "")
    __strings__ = []
    s = re.sub("(?u)([%s])([^\n%s]*)(?<!\\\\)[%s]" % (lq, rq, rq), __encodestring__, s)
    s = re.sub('(?u)(?<![0-9])(")(~?\w*)', __encodestring__, s)

    # remove extra spaces
    s = chsp.sub(" ", s)

    # remove inline comments
    s = re.sub(r"[ ]*;[^\n]*", "", s)

    # n-dash and m-dash as minus signs
    s = re.sub(r"(?u)[–—]", "-", s)

    # replace procedure names
    s = re.sub(r"(?i)^[ ]*(%s)[ ]+" % __l12n__(_.lng)['TO'], "__def__ ", s)
    s = re.sub(r"(?i)\n[ ]*(%s)[ ]+" % __l12n__(_.lng)['TO'], "\n__def__ ", s)
    subnames = re.findall(u"(?iu)(?<=__def__ )\w+", s)
    globs = ""
    functions = ["range", "__int__", "__float__", "Random", "Input", "__string__", "len", "round", "abs", "sin", "cos", "sqrt", "log10", "set", "list", "tuple", "re.sub", "re.search", "re.findall", "sorted", "min", "max"]

    if len(subnames) > 0:
        globs = "global %s" % ", ".join(subnames)
        # search user functions (function calls with two or more arguments need explicite Python parentheses)
        ends = __l12n__(_.lng)["END"] # support multiple names of "END"
        firstend = ends.split("|")[0]
        s = re.sub(r"(?<!:)\b(?:%s)\b" % ends, firstend, s)
        __l12n__(_.lng)["END"] = firstend
        functions += [ re.findall("(?u)\w+",i[0])[0]  for i in re.findall(r"""(?iu)(?<=__def__ )([^\n]*)\n # beginning of a procedure
            (?:[^\n]*(?<!\b(%(END)s))\n)* # 0 or more lines (not END)
            [^\n]*\b(?:%(OUTPUT)s)\b[^\n]*\n # line with OUTPUT (functions = procedures with OUTPUT)
            (?:[^\n]*(?<!\b(?:%(END)s))\n)* # 0 or more lines (not END)
            [ \t]*\b(?:%(END)s)\b""" % __l12n__(_.lng), s, re.X) ]
        __l12n__(_.lng)["END"] = ends
        # add line breaks before procedure calls
        procedures = set(subnames) - set(functions)
        if len(procedures) > 0:
            s = re.sub(r"(?<!__def__)(?<![-+=*/])(?<!%s)(?:^|[ \t]+)(" % ")(?<!".join(functions) + "|".join(procedures) + ")(?!\w)", r"\n\1", s)

    # compile native Logo
    for i in __comp__[_.lng]:
        s = re.sub(u"(?u)" + i[0], i[1], s)
    indent = 0
    result = ""
    func = re.compile("(?iu)(def (\w+))(\(.*\):)")
    expr = r"""(?iu)(?<!def[ ])(?<![:\w])%(name)s(?!\w)(?!\()(?![ ]\()
        (
            ([ ]+\[*([-+]|\([ ]?)*((%(functions)s)\b[ ]*\(*)*
            (?:0x[0-9a-f]+|[0-9]+([,.][0-9]+)?|:?\w+(?:[.]\w+[\(]?[\)]?)?]*|\[])]*[\)]*
            (
                (?:[ ]*([+*/,<>]|//|==|<=|>=|<>|!=)[ ]*|[ ]*-[ ]+|-|[ ]*[*][*][ ]*) # operators, eg. "**", " - ", "-", "- "
                \[*([-+]|\([ ]?)* # minus sign, parenthesis
                ((%(functions)s)\b[ ]*\(*)*(0x[0-9a-f]+|[0-9]+([.,][0-9]+)?|:?\w+(?:[.]\w+[\(]?[\)]?)?)]*
            ([ ]?\))*)*
        [\)]*){,%(repeat)s}
    )
"""
    chargsp = re.compile(r"(?<![\(,])(?<!%s) (?!\)|,)" % ")(?<!".join(functions))

    # compile to Python
    joinfunc = "|".join(functions)
    funcnames = {}

    for i in s.split("\n"):
        i = i.strip()
        if i[0:4] == 'def ':
            s = func.search(i)
            if s.group(3) == '():':
                names[s.group(2)] = (0, "")
            else:
                s2 = len(chsp.findall(s.group(3))) + 1
                i = s.group(1) + chsp.sub(", ", s.group(3))
                names[s.group(2)] = (s2, re.compile(expr % {"name": s.group(2), "functions": joinfunc, "repeat": s2}, re.X))
        for j in functions:
            if j in i:
                if not j in funcnames:
                    funcnames[j] = (1, re.compile(expr % {"name": j, "functions": joinfunc, "repeat": 1 + 2 * int(j == 'range')}, re.X))
                r = funcnames[j][1].search(i)
                while r:
                    i = i[:r.start()] + j + '(' + chargsp.sub(", ", rmsp.sub(lambda l: l.group(1).strip(), r.group(1).strip())) + ')' + i[r.end():]
                    i = parenfix.sub("\\1)]", i)
                    r = funcnames[j][1].search(i)
        for j in names:
            if j in i:
                if names[j][0] == 0:
                    if not j in functions:
                        i = re.sub(r"(?iu)(?<!def )(?<![_\w])\b%s\b(?!\w)" %j, j+'()', i)
                else:
                    r = names[j][1].search(i)
                    if r:
                        i = i[:r.start()] + j + '(' + chargsp.sub(", ", rmsp.sub(lambda l: l.group(1).strip(), r.group(1).strip())) + ')' + i[r.end():]
                        i = parenfix.sub("\\1)]", i)
        if i[0:1] == '[':
            i = i[1:]
            indent += 1
            result = result + "\n" + " " * indent + "__checkhalt__()\n"
        if i[0:1] == ')':
            i = i[1:] + ')'
        result = result + "\n" + " " * indent + i
        if i[0:1] == ']':
            result = result[:-1]
            indent -= 1

    # colon_to_underline in Logo variables
    result = chch.sub("_", result)

    # character encoding
    result = to_ascii(result).replace(r"\n", "\n")

    # decode strings
    result = re.sub(__DECODE_STRING_REGEX__, __decodestring__, result)
    return to_ascii(globs) + "\n" + result

def __gotoline__(n):
    _.cursor.collapseToStart()
    for i in range(1, n):
        _.cursor.gotoNextParagraph(False)
    try:
        _.doc.CurrentController.getViewCursor().gotoRange(_.cursor, False)
    except:
        __dispatcher__(".uno:Escape")
        _.doc.CurrentController.getViewCursor().gotoRange(_.cursor.getStart(), False)

g_exportedScripts = left, right, goforward, gobackward, run, stop, home, clearscreen, commandline, __translate__

# vim: set noet sw=4 ts=4:
