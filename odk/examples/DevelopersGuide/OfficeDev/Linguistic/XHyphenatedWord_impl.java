/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.lang.Locale;

public class XHyphenatedWord_impl implements
    com.sun.star.linguistic2.XHyphenatedWord
{
    private String     aWord;
    private String     aHyphenatedWord;
    private final short      nHyphenPos;
    private final short      nHyphenationPos;
    private Locale     aLang;
    private final boolean  bIsAltSpelling;

    public XHyphenatedWord_impl(
            String      aWord,
            Locale      aLang,
            short       nHyphenationPos,
            String      aHyphenatedWord,
            short       nHyphenPos )
    {
        this.aWord = aWord;
        this.aLang = aLang;
        this.nHyphenationPos = nHyphenationPos;
        this.aHyphenatedWord = aHyphenatedWord;
        this.nHyphenPos = nHyphenPos;
        this.bIsAltSpelling = (aWord != aHyphenatedWord);

        //!! none of these cases should ever occur!
        //!! values provided only for safety
        if (this.aWord == null)
            this.aWord = "";
        if (this.aLang == null)
            this.aLang = new Locale();
        if (this.aHyphenatedWord == null)
            this.aHyphenatedWord = "";
    }


    // XHyphenatedWord
    public String getWord() throws com.sun.star.uno.RuntimeException
    {
        return aWord;
    }
    public Locale getLocale() throws com.sun.star.uno.RuntimeException
    {
        return aLang;
    }
    public short getHyphenationPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenationPos;
    }
    public String getHyphenatedWord() throws com.sun.star.uno.RuntimeException
    {
        return aHyphenatedWord;
    }
    public short getHyphenPos() throws com.sun.star.uno.RuntimeException
    {
        return nHyphenPos;
    }
    public boolean isAlternativeSpelling() throws com.sun.star.uno.RuntimeException
    {
        return bIsAltSpelling;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
