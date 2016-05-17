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

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.UnsupportedFlavorException;
import com.sun.star.datatransfer.XTransferable;
import com.sun.star.uno.Type;


// A simple transferable containing only
// one format, unicode text


public class TextTransferable implements XTransferable
{
    public TextTransferable(String aText)
    {
        text = aText;
    }

    // XTransferable methods

    public Object getTransferData(DataFlavor aFlavor) throws UnsupportedFlavorException
    {
        if ( !aFlavor.MimeType.equalsIgnoreCase( UNICODE_CONTENT_TYPE ) )
            throw new UnsupportedFlavorException();

        return text;
    }

    public DataFlavor[] getTransferDataFlavors()
    {
        DataFlavor[] adf = new DataFlavor[1];

        DataFlavor uniflv = new DataFlavor(
            UNICODE_CONTENT_TYPE,
            "Unicode Text",
            new Type(String.class) );

        adf[0] = uniflv;

        return adf;
    }

    public boolean isDataFlavorSupported(DataFlavor aFlavor)
    {
        return aFlavor.MimeType.equalsIgnoreCase(UNICODE_CONTENT_TYPE);
    }

// members

    private final String text;
    private static final String UNICODE_CONTENT_TYPE = "text/plain;charset=utf-16";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
