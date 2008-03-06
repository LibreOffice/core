import sys, os.path
from globals import *
import macroparser

class EOF(Exception):
    def __init__ (self):
        pass

    def str (self):
        return "end of file"

class BOF(Exception):
    def __init__ (self):
        pass

    def str (self):
        return "beginning of file"


def removeHeaderQuotes (orig):
    if len(orig) <= 2:
        return orig
    elif orig[0] == orig[-1] == '"':
        return orig[1:-1]
    elif orig[0] == '<' and orig[-1] == '>':
        return orig[1:-1]

    return orig


def dumpTokens (tokens, toError=False):

    scope = 0
    indent = "    "
    line = ''
    chars = ''
    
    for token in tokens:
        if token in '{<':
            if len(line) > 0:
                chars += indent*scope + line + "\n"
                line = ''
            chars += indent*scope + token + "\n"
            scope += 1

        elif token in '}>':
            if len(line) > 0:
                chars += indent*scope + line + "\n"
                line = ''
            scope -= 1
            chars += indent*scope + token

        elif token == ';':
            if len(line) > 0:
                chars += indent*scope + line + ";\n"
                line = ''
            else:
                chars += ";\n"
        elif len(token) > 0:
            line += token + ' '

    if len(line) > 0:
        chars += line
    chars += "\n"
    if toError:
        sys.stderr.write(chars)
    else:
        sys.stdout.write(chars)


class HeaderData(object):
    def __init__ (self):
        self.defines = {}
        self.tokens = []


class SrcLexer(object):
    """Lexicographical analyzer for .src format.

The role of a lexer is to parse the source file and break it into 
appropriate tokens.  Such tokens are later passed to a parser to
build the syntax tree.
"""
    headerCache = {}

    VISIBLE = 0
    INVISIBLE_PRE = 1
    INVISIBLE_POST = 2

    def __init__ (self, chars, filepath = None):
        self.filepath = filepath
        self.parentLexer = None
        self.chars = chars
        self.bufsize = len(self.chars)

        # TODO: use parameters for this
        # Properties that can be copied.
        self.headerDict = dict ()
        self.debug = False
        self.debugMacro = False
        self.includeDirs = list ()
        self.expandHeaders = True
        self.inMacroDefine = False
        self.stopOnHeader = False

    def copyProperties (self, other):
        """Copy properties from another instance of SrcLexer."""

        # TODO: use parameters for this
        self.headerDict = other.headerDict
        self.debug = other.debug
        self.debugMacro = other.debugMacro
        self.includeDirs = other.includeDirs[:]
        self.expandHeaders = other.expandHeaders
        self.inMacroDefine = other.inMacroDefine
        self.stopOnHeader = other.stopOnHeader

    def init (self):
        self.firstNonBlank = ''
        self.token = ''
        self.tokens = []
        self.defines = {}
        self.visibilityStack = []

    def getTokens (self):
        return self.tokens

    def getDefines (self):
        return self.defines

    def nextPos (self, i):
        while True:
            i += 1
            try:
                c = self.chars[i]
            except IndexError:
                raise EOF

            if ord(c) in [0x0D]:
                continue
            break
        return i

    def prevPos (self, i):
        while True:
            i -= 1
            try:
                c = self.chars[i]
            except IndexError:
                raise BOF

            if ord(c) in [0x0D]:
                continue
            break
        return i

    def isCodeVisible (self):
        if len(self.visibilityStack) == 0:
            return True
        for item in self.visibilityStack:
            if item != SrcLexer.VISIBLE:
                return False
        return True
        
    def tokenize (self):
        self.init()

        i = 0
        while True:
            c = self.chars[i]

            if self.firstNonBlank == '' and not c in [' ', "\n", "\t"]:
                # Store the first non-blank in a line.
                self.firstNonBlank = c
            elif c == "\n":
                self.firstNonBlank = ''

            if c == '#':
                i = self.pound(i)
            elif c == '/':
                i = self.slash(i)
            elif c == "\n":
                i = self.lineBreak(i)
            elif c == '"':
                i = self.doubleQuote(i)
            elif c in [' ', "\t"]:
                i = self.blank(i)
            elif c in ";()[]{}<>,=+-*":
                # Any outstanding single-character token.
                i = self.anyToken(i, c)
            elif self.isCodeVisible():
                self.token += c

            try:
                i = self.nextPos(i)
            except EOF:
                break

        if len(self.token):
            self.tokens.append(self.token)

        if not self.parentLexer and self.debug:
            progress ("-"*68 + "\n")
            progress ("All defines found in this translation unit:\n")
            keys = self.defines.keys()
            keys.sort()
            for key in keys:
                progress ("@ %s\n"%key)

    def dumpTokens (self, toError=False):
        dumpTokens(self.tokens, toError)


    def maybeAddToken (self):
        if len(self.token) > 0:
            self.tokens.append(self.token)
            self.token = ''


    #--------------------------------------------------------------------
    # character handlers

    def blank (self, i):
        if not self.isCodeVisible():
            return i

        self.maybeAddToken()
        return i


    def pound (self, i):

        if self.inMacroDefine:
            return i

        if not self.firstNonBlank == '#':
            return i

        self.maybeAddToken()
        # We are in preprocessing mode.

        # Get the macro command name '#<command> .....'

        command, define, buf = '', '', ''
        firstNonBlank = False
        while True:
            try:
                i = self.nextPos(i)
                c = self.chars[i]
                if c == '\\' and self.chars[self.nextPos(i)] == "\n":
                    i = self.nextPos(i)
                    continue
            except EOF:
                break

            if c == "\n":
                if len(buf) > 0 and len(command) == 0:
                    command = buf
                i = self.prevPos(i)
                break
            elif c in [' ', "\t"]:
                if not firstNonBlank:
                    # Ignore any leading blanks after the '#'.
                    continue

                if len(command) == 0:
                    command = buf
                    buf = ''
                else:
                    buf += ' '
            elif c == '(':
                if len(buf) > 0 and len(command) == 0:
                    command = buf
                buf += c
            else:
                if not firstNonBlank:
                    firstNonBlank = True
                buf += c

        if command == 'define':
            self.handleMacroDefine(buf)
        elif command == 'include':
            self.handleMacroInclude(buf)
        elif command == 'ifdef':
            defineName = buf.strip()
            if self.defines.has_key(defineName):
                self.visibilityStack.append(SrcLexer.VISIBLE)
            else:
                self.visibilityStack.append(SrcLexer.INVISIBLE_PRE)

        elif command == 'ifndef':
            defineName = buf.strip()
            if self.defines.has_key(defineName):
                self.visibilityStack.append(SrcLexer.INVISIBLE_PRE)
            else:
                self.visibilityStack.append(SrcLexer.VISIBLE)

        elif command == 'if':
            if self.evalCodeVisibility(buf):
                self.visibilityStack.append(SrcLexer.VISIBLE)
            else:
                self.visibilityStack.append(SrcLexer.INVISIBLE_PRE)

        elif command == 'elif':
            if len(self.visibilityStack) == 0:
                raise ParseError ('')

            if self.visibilityStack[-1] == SrcLexer.VISIBLE:
                self.visibilityStack[-1] = SrcLexer.INVISIBLE_POST
            elif self.visibilityStack[-1] == SrcLexer.INVISIBLE_PRE:
                # Evaluate only if the current visibility is false.
                if self.evalCodeVisibility(buf):
                    self.visibilityStack[-1] = SrcLexer.VISIBLE

        elif command == 'else':
            if len(self.visibilityStack) == 0:
                raise ParseError ('')

            if self.visibilityStack[-1] == SrcLexer.VISIBLE:
                self.visibilityStack[-1] = SrcLexer.INVISIBLE_POST
            if self.visibilityStack[-1] == SrcLexer.INVISIBLE_PRE:
                self.visibilityStack[-1] = SrcLexer.VISIBLE

        elif command == 'endif':
            if len(self.visibilityStack) == 0:
                raise ParseError ('')
            self.visibilityStack.pop()

        elif command == 'undef':
            pass
        elif command in ['error', 'pragma']:
            pass
        else:
            print "'%s' '%s'"%(command, buf)
            print self.filepath
            sys.exit(0)

        return i


    def evalCodeVisibility (self, buf):
        try:
            return eval(buf)
        except:
            return True
        
    def handleMacroDefine (self, buf):

        mparser = macroparser.MacroParser(buf)
        mparser.debug = self.debugMacro
        mparser.parse()
        macro = mparser.getMacro()
        if macro:
            self.defines[macro.name] = macro

    def handleMacroInclude (self, buf):

        # Strip excess string if any.
        pos = buf.find(' ')
        if pos >= 0:
            buf = buf[:pos]
        headerSub = removeHeaderQuotes(buf)

        if not self.expandHeaders:
            # We don't want to expand headers.  Bail out.
            if self.debug:
                progress ("%s ignored\n"%headerSub)
            return

        defines = {}
        headerPath = None
        for includeDir in self.includeDirs:
            hpath = includeDir + '/' + headerSub
            if os.path.isfile(hpath) and hpath != self.filepath:
                headerPath = hpath
                break

        if not headerPath:
            error("included header file " + headerSub + " not found\n", self.stopOnHeader)
            return
        
        if self.debug:
            progress ("%s found\n"%headerPath)

        if headerPath in self.headerDict:
            if self.debug:
                progress ("%s already included\n"%headerPath)
            return

        if SrcLexer.headerCache.has_key(headerPath):
            if self.debug:
                progress ("%s in cache\n"%headerPath)
            for key in SrcLexer.headerCache[headerPath].defines.keys():
                self.defines[key] = SrcLexer.headerCache[headerPath].defines[key]
            return

        chars = open(headerPath, 'r').read()
        mclexer = SrcLexer(chars, headerPath)
        mclexer.copyProperties(self)
        mclexer.parentLexer = self
        mclexer.tokenize()
        hdrData = HeaderData()
        hdrData.tokens = mclexer.getTokens()
        headerDefines = mclexer.getDefines()
        for key in headerDefines.keys():
            defines[key] = headerDefines[key]
            hdrData.defines[key] = headerDefines[key]

        self.headerDict[headerPath] = True
        SrcLexer.headerCache[headerPath] = hdrData

        # Update the list of headers that have already been expaneded.
        for key in mclexer.headerDict.keys():
            self.headerDict[key] = True

        if self.debug:
            progress ("defines found in header %s:\n"%headerSub)
            for key in defines.keys():
                progress ("  '%s'\n"%key)

        for key in defines.keys():
            self.defines[key] = defines[key]


    def slash (self, i):
        if not self.isCodeVisible():
            return i

        if i < self.bufsize - 1 and self.chars[i+1] == '/':
            # Parse line comment.
            line = ''
            i += 2
            while i < self.bufsize:
                c = self.chars[i]
                if ord(c) in [0x0A, 0x0D]:
                    return i - 1
                line += c
                i += 1
            self.token = ''
        elif i < self.bufsize - 1 and self.chars[i+1] == '*':
            comment = ''
            i += 2
            while i < self.bufsize:
                c = self.chars[i]
                if c == '/' and self.chars[i-1] == '*':
                    return i
                comment += c
                i += 1
        else:
            return self.anyToken(i, '/')

        return i


    def lineBreak (self, i):
        if not self.isCodeVisible():
            return i

        self.maybeAddToken()

        return i


    def doubleQuote (self, i):
        if not self.isCodeVisible():
            return i

        literal = ''
        i += 1
        while i < self.bufsize:
            c = self.chars[i]
            if c == '"':
                self.tokens.append('"'+literal+'"')
                break
            literal += c
            i += 1

        return i


    def anyToken (self, i, token):
        if not self.isCodeVisible():
            return i

        self.maybeAddToken()
        self.token = token
        self.maybeAddToken()
        return i
