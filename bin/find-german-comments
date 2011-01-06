#!/usr/bin/env python
########################################################################
#
#  Copyright (c) 2010 Jonas Jensen, Miklos Vajna
#
#  Permission is hereby granted, free of charge, to any person
#  obtaining a copy of this software and associated documentation
#  files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use,
#  copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following
#  conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
########################################################################


import sys, re, subprocess, os, optparse, string

class Parser:
    """
    This parser extracts comments from source files, tries to guess
    their language and then prints out the german ones.
    """
    def __init__(self):
        self.strip = string.punctuation + " \n"
        op = optparse.OptionParser()
        op.set_usage("%prog [options] <rootdir>\n\n" +
            "Searches for german comments in cxx/hxx source files inside a given root\n" +
            "directory recursively.")
        op.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False,
            help="Turn on verbose mode (print progress to stderr)")
        self.options, args = op.parse_args()
        try:
            dir = args[0]
        except IndexError:
            dir = "."
        self.check_source_files(dir)

    def get_comments(self, filename):
        """
        Extracts the source code comments.
        """
        linenum = 0
        if self.options.verbose:
            sys.stderr.write("processing file '%s'...\n" % filename)
        sock = open(filename)
        # add an empty line to trigger the output of collected oneliner
        # comment group
        lines = sock.readlines() + ["\n"]
        sock.close()

        in_comment = False
        buf = []
        count = 1
        for i in lines:
            if "//" in i and not in_comment:
                # if we find a new //-style comment, then we
                # just append it to a previous one if: there is
                # only whitespace before the // mark that is
                # necessary to make comments longer, giving
                # more reliable output
                if not len(re.sub("(.*)//.*", r"\1", i).strip(self.strip)):
                    s = re.sub(".*// ?", "", i).strip(self.strip)
                    if len(s):
                        buf.append(s)
                else:
                    # otherwise it's an independent //-style comment in the next line
                    yield (count, "\n    ".join(buf))
                    buf = [re.sub(".*// ?", "", i.strip(self.strip))]
            elif "//" not in i and not in_comment and len(buf) > 0:
                # first normal line after a // block
                yield (count, "\n    ".join(buf))
                buf = []
            elif "/*" in i and "*/" not in i and not in_comment:
                # start of a real multiline comment
                in_comment = True
                linenum = count
                s = re.sub(".*/\*+", "", i.strip(self.strip))
                if len(s):
                    buf.append(s.strip(self.strip))
            elif in_comment and not "*/" in i:
                # in multiline comment
                s = re.sub("^( |\|)*\*?", "", i)
                if len(s.strip(self.strip)):
                    buf.append(s.strip(self.strip))
            elif "*/" in i and in_comment:
                # end of multiline comment
                in_comment = False
                s = re.sub(r"\*+/.*", "", i.strip(self.strip))
                if len(s):
                    buf.append(s)
                yield (count, "\n    ".join(buf))
                buf = []
            elif "/*" in i and "*/" in i:
                # c-style oneliner comment
                yield (count, re.sub(".*/\*(.*)\*/.*", r"\1", i).strip(self.strip))
            count += 1

    def get_lang(self, s):
        """ the output is 'german' or 'english' or 'german or english'. when
        unsure, just don't warn, there are strings where you just can't
        teremine the results reliably, like '#110680#' """
        cwd = os.getcwd()
        # change to our directory
        os.chdir(os.path.split(os.path.abspath(sys.argv[0]))[0])
        sock = subprocess.Popen(["text_cat/text_cat", "-d", "text_cat/LM"], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        sock.stdin.write(s)
        sock.stdin.close()
        lang = sock.stdout.read().strip()
        sock.stdout.close()
        os.chdir(cwd)
        return lang

    def is_german(self, s):
        """
        determines if a string is german or not
        """
        # for short strings we can't do reliable recognition, so skip
        # short strings and less than 4 words
        s = s.replace('\n', ' ')
        if len(s) < 32 or len(s.split()) < 4:
            return False
        return "german" == self.get_lang(s)

    def check_file(self, path):
        """
        checks each comment in a file
        """
        for linenum, s in self.get_comments(path):
            if self.is_german(s):
                print "%s:%s: %s" % (path, linenum, s)

    def check_source_files(self, dir):
        """
        checks each _tracked_ file in a directory recursively
        """
        sock = os.popen(r"git ls-files '%s' |egrep '\.(c|h)xx$'" % dir)
        lines = sock.readlines()
        sock.close()
        for path in lines:
            self.check_file(path.strip())

try:
    Parser()
except KeyboardInterrupt:
    print "Interrupted!"
    sys.exit(0)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
