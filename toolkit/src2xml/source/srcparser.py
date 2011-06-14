import sys
from globals import *
import srclexer

# simple name translation map
postTransMap = {"ok-button": "okbutton", 
                "cancel-button": "cancelbutton",
                "help-button": "helpbutton"}

def transName (name):
    """Translate a mixed-casing name to dash-separated name.

Translate a mixed-casing name (e.g. MyLongName) to a dash-separated name
(e.g. my-long-name).
"""
    def isUpper (c):
        return c >= 'A' and c <= 'Z'

    newname = ''
    parts = []
    buf = ''
    for c in name:
        if isUpper(c) and len(buf) > 1:
            parts.append(buf)
            buf = c
        else:
            buf += c

    if len(buf) > 0:
        parts.append(buf)

    first = True
    for part in parts:
        if first:
            first = False
        else:
            newname += '-'
        newname += part.lower()

    # special-case mapping ...
    if 0: #postTransMap.has_key(newname):
        newname = postTransMap[newname]

    return newname


def transValue (value):
    """Translate certain values.

Examples of translated values include TRUE -> true, FALSE -> false.
"""
    if value.lower() in ["true", "false"]:
        value = value.lower()
    return value


def renameAttribute (name, elemName):

    # TODO: all manner of evil special cases ...
    if elemName == 'metric-field' and name == 'spin-size':
        return 'step-size'

    return name


class Statement(object):
    """Container to hold information for a single statement.

Each statement consists of the left-hand-side token(s), and right-hand-side
tokens, separated by a '=' token.  This class stores the information on the
left-hand-side tokens.
"""
    def __init__ (self):
        self.leftTokens = []
        self.leftScope = None


class MacroExpander(object):
    def __init__ (self, tokens, defines):
        self.tokens = tokens
        self.defines = defines

    def expand (self):
        self.pos = 0
        while self.pos < len(self.tokens):
            self.expandToken()

    def expandToken (self):
        token = self.tokens[self.pos]
        if not self.defines.has_key(token):
            self.pos += 1
            return

        macro = self.defines[token]
        nvars = len(macro.vars.keys())
        if nvars == 0:
            # Simple expansion
            self.tokens[self.pos:self.pos+1] = macro.tokens
            return
        else:
            # Expansion with arguments.
            values, lastPos = self.parseValues()
            newtokens = []
            for mtoken in macro.tokens:
                if macro.vars.has_key(mtoken):
                    # variable
                    pos = macro.vars[mtoken]
                    valtokens = values[pos]
                    for valtoken in valtokens:
                        newtokens.append(valtoken)
                else:
                    # not a variable
                    newtokens.append(mtoken)

            self.tokens[self.pos:self.pos+lastPos+1] = newtokens


    def parseValues (self):
        """Parse tokens to get macro function variable values.

Be aware that there is an implicit quotes around the text between the open 
paren, the comma(s), and the close paren.  For instance, if a macro is defined 
as FOO(a, b) and is used as FOO(one two three, and four), then the 'a' must be 
replaced with 'one two three', and the 'b' replaced with 'and four'.  In other 
words, whitespace does not end a token.  

"""
        values = []
        i = 1
        scope = 0
        value = []
        while True:
            try:
                tk = self.tokens[self.pos+i]
            except IndexError:
                progress ("error parsing values (%d)\n"%i)
                for j in xrange(0, i):
                    print self.tokens[self.pos+j],
                print ''
                srclexer.dumpTokens(self.tokens)
                srclexer.dumpTokens(self.newtokens)
                print "tokens expanded so far:"
                for tk in self.expandedTokens:
                    print "-"*20
                    print tk
                    srclexer.dumpTokens(self.defines[tk].tokens)
                sys.exit(1)
            if tk == '(':
                value = []
                scope += 1
            elif tk == ',':
                values.append(value)
                value = []
            elif tk == ')':
                scope -= 1
                values.append(value)
                value = []
                if scope == 0:
                    break
                else:
                    raise ParseError ('')
            else:
                value.append(tk)
            i += 1

        return values, i

    def getTokens (self):
        return self.tokens


class SrcParser(object):

    def __init__ (self, tokens, defines = None):
        self.tokens = tokens
        self.defines = defines
        self.debug = False
        self.onlyExpandMacros = False

    def init (self):
        self.elementStack = [RootNode()]
        self.stmtData = Statement()
        self.tokenBuf = []
        self.leftTokens = []

        # Expand defined macros.
        if self.debug:
            progress ("-"*68+"\n")
            for key in self.defines.keys():
                progress ("define: %s\n"%key)

        self.expandMacro()
        self.tokenSize = len(self.tokens)

    def expandMacro (self):
        macroExp = MacroExpander(self.tokens, self.defines)
        macroExp.expand()
        self.tokens = macroExp.getTokens()
        if self.onlyExpandMacros:
            srclexer.dumpTokens(self.tokens)
            sys.exit(0)

    def parse (self):
        """Parse it!

This is the main loop for the parser.  This is where it all begins and ends.
"""
        self.init()

        i = 0
        while i < self.tokenSize:
            tk = self.tokens[i]
            if tk == '{':
                i = self.openBrace(i)
            elif tk == '}':
                i = self.closeBrace(i)
            elif tk == ';':
                i = self.semiColon(i)
            elif tk == '=':
                i = self.assignment(i)
            else:
                self.tokenBuf.append(tk)

            i += 1

        return self.elementStack[0]

    #-------------------------------------------------------------------------
    # Token Handlers

    """
Each token handler takes the current token position and returns the position 
of the last token processed.  For the most part, the current token position 
and the last processed token are one and the same, in which case the handler 
can simply return the position value it receives without incrementing it.  

If you need to read ahead to process more tokens than just the current token, 
make sure that the new token position points to the last token that has been 
processed, not the next token that has not yet been processed.  This is 
because the main loop increments the token position when it returns from the 
handler.
""" 

    # assignment token '='
    def assignment (self, i):
        self.leftTokens = self.tokenBuf[:]
        if self.stmtData.leftScope == None:
            # Keep track of lhs data in case of compound statement.
            self.stmtData.leftTokens = self.tokenBuf[:]
            self.stmtData.leftScope = len(self.elementStack) - 1

        self.tokenBuf = []
        return i
    
    # open brace token '{'
    def openBrace (self, i):
        bufSize = len(self.tokenBuf)
        leftSize = len(self.leftTokens)
        obj = None
        if bufSize == 0 and leftSize > 0:
            # Name = { ...
            obj = Element(self.leftTokens[0])

        elif bufSize > 0 and leftSize == 0:
            # Type Name { ...
            wgtType = self.tokenBuf[0]
            wgtRID = None
            if bufSize >= 2:
                wgtRID = self.tokenBuf[1]
            obj = Element(wgtType, wgtRID)

        else:
            # LeftName = Name { ...
            obj = Element(self.leftTokens[0])
            obj.setAttr("type", self.tokenBuf[0])

        obj.name = transName(obj.name)

        if obj.name == 'string-list':
            i = self.parseStringList(i)
        elif obj.name == 'filter-list':
            i = self.parseFilterList(i, obj)
        else:
            self.elementStack[-1].appendChild(obj)
            self.elementStack.append(obj)

        self.tokenBuf = []
        self.leftTokens = []

        return i

    # close brace token '}'
    def closeBrace (self, i):
        if len(self.tokenBuf) > 0:
            if self.debug:
                print self.tokenBuf
            raise ParseError ('')
        self.elementStack.pop()
        return i

    # semi colon token ';'
    def semiColon (self, i):
        stackSize = len(self.elementStack)
        scope = stackSize - 1
        if len(self.tokenBuf) == 0:
            pass
        elif scope == 0:
            # We are not supposed to have any statment in global scope.  
            # Just ignore this statement.
            pass
        else:
            # Statement within a scope.  Import it as an attribute for the 
            # current element.
            elem = self.elementStack[-1]

            name = "none"
            if len(self.leftTokens) > 0:
                # Use the leftmost token as the name for now.  If we need to
                # do more complex parsing of lhs, add more code here.
                name = self.leftTokens[0]
                name = transName(name)

            if name == 'pos':
                i = self.parsePosAttr(i)
            elif name == 'size':
                i = self.parseSizeAttr(i)
            elif len (self.tokenBuf) == 1:
                # Simple value
                value = transValue(self.tokenBuf[0])
                name = renameAttribute(name, elem.name)
                elem.setAttr(name, value)

            if not self.stmtData.leftScope == None and self.stmtData.leftScope < scope:
                # This is a nested scope within a statement.  Do nothing for now.
                pass

        if self.stmtData.leftScope == scope:
            # end of (nested) statement.
            self.stmtData.leftScope = None

        self.tokenBuf = []
        self.leftTokens = []

        return i

    def parseStringList (self, i):

        i += 1
        while i < self.tokenSize:
            tk = self.tokens[i]
            if tk == '}':
                break
            i += 1

        return i

    def parseFilterList (self, i, obj):
        self.elementStack[-1].appendChild(obj)
        self.elementStack.append(obj)

        return i

    def parsePosAttr (self, i):

        # MAP_APPFONT ( 6 , 5 )
        elem = self.elementStack[-1]
        x, y = self.parseMapAppfont(self.tokenBuf)
        elem.setAttr("x", x)
        elem.setAttr("y", y)

        return i

    def parseSizeAttr (self, i):

        # MAP_APPFONT ( 6 , 5 )
        elem = self.elementStack[-1]
        width, height = self.parseMapAppfont(self.tokenBuf)
        elem.setAttr("width", width)
        elem.setAttr("height", height)

        return i

    def parseMapAppfont (self, tokens):
        values = []
        scope = 0
        val = ''
        for tk in tokens:
            if tk == '(':
                scope += 1
                if scope == 1:
                    val = ''
                else:
                    val += tk
            elif tk == ')':
                scope -= 1
                if scope == 0:
                    if len(val) == 0:
                        raise ParseError ('')
                    values.append(val)
                    break
                else:
                    val += tk
            elif tk == ',':
                if len(val) == 0:
                    raise ParseError ('')
                values.append(val)
                val = ''
            elif scope > 0:
                val += tk

        if len(values) != 2:
            raise ParseError ('')

        return eval(values[0]), eval(values[1])


