// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-

// Copyright (C) 2011 Tor Lillqvist <tml@iki.fi>
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

package fi.iki.tml;

import java.util.*;

public class CommandLine
    implements Iterable<String>
{
    private String commandLine;

    public class Tokenizer
        implements Iterator<String>
    {
        private int index = 0;
        private String savedNext = null;

        public Tokenizer()
        {
        }

        public boolean hasNext()
        {
            if (savedNext != null)
                return true;
            try {
                savedNext = next();
                return true;
            }
            catch (NoSuchElementException e) {
            }
            return false;
        }

        public String next()
        {
            if (savedNext != null) {
                String result = savedNext;
                savedNext = null;
                return result;
            }

            StringBuffer sb = new StringBuffer(commandLine.length());

            while (index < commandLine.length() &&
                   commandLine.charAt(index) == ' ')
                index++;

            if (index == commandLine.length())
                throw new NoSuchElementException();

            while (index < commandLine.length() &&
                   commandLine.charAt(index) != ' ') {
                char c = commandLine.charAt(index);
                if (c == '\'') {
                    index++;
                    while (index < commandLine.length() &&
                           commandLine.charAt(index) != '\'') {
                        sb.append(commandLine.charAt(index));
                        index++;
                    }
                } else if (c == '"') {
                    index++;
                    while (index < commandLine.length() &&
                           commandLine.charAt(index) != '\"') {
                        if (commandLine.charAt(index) == '\\') {
                            index++;
                            if (index < commandLine.length())
                                sb.append(commandLine.charAt(index));
                        } else {
                            sb.append(commandLine.charAt(index));
                        }
                        index++;
                    }
                } else if (c == '\\') {
                    index++;
                    if (index < commandLine.length())
                        sb.append(commandLine.charAt(index));
                } else {
                    sb.append(c);
                }
                index++;
            }
            return sb.toString();
        }

        public void remove()
        {
            throw new UnsupportedOperationException();
        }
    }

    public Iterator<String> iterator()
    {
        return new Tokenizer();
    }

    public CommandLine(String commandLine)
    {
        this.commandLine = commandLine;
    }

    public String[] split()
    {
        ArrayList<String> argv = new ArrayList<String>(10);

        try {
            for (String s : this)
                argv.add(s);
        }
        catch (NoSuchElementException e) {
        }

        return argv.toArray(new String[0]);
    }

    public static String[] split(String commandLine)
    {
        return new CommandLine(commandLine).split();
    }

    public static void main(String[] args)
    {
        class Test
        {
            Test(String commandLine)
            {
                String[] argv = split(commandLine);
                System.out.println("Split of " + commandLine + ":");
                int n = 0;
                for (String s : argv) {
                    System.out.println("argv[" + n + "}: length " + s.length() + ": \"" + s + "\"");
                    n++;
                }
            }
        }

        new Test("");
        new Test(" ");
        new Test("  ");
        new Test("  '' ");
        new Test("abc def");
        new Test("abc '' def");
        new Test("abc \"\" def");
        new Test(" abc def ");
        new Test("  abc  def  ");
        new Test("abc\" \"def");
        new Test("abc\" \"def \"gh\\i\\\" jkl\"");
        new Test("abc' def' '\\ghi jkl'");
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
