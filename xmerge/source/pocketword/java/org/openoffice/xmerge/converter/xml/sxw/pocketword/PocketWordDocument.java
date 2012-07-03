/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.converter.xml.ParaStyle;
import org.openoffice.xmerge.converter.xml.TextStyle;


/**
 * <p>Class representing a Pocket Word Document.</p>
 *
 * <p><code>PocketWordDocument</code> is used to create new Pocket Word documents
 *    and to read existing data to allow for conversion to OpenOffice Writer
 *    format.</p>
 *
 * @author  Mark Murnane
 * @version 1.1
 */
public class PocketWordDocument implements Document, PocketWordConstants {
    private String      docName;

    private byte[] preamble;
    private ArrayList<Object> fonts;
    private DocumentDescriptor descriptor;
    private ArrayList<Paragraph> paragraphs;

    private Paragraph   currentPara;

    /*
     * The trailer currently appears to be constant, but if its found to
     * have a variable component, then this initialisation should be moved
     * to an initTrailer() method.
     *
     * Padding is sometimes needed before the trailer to ensure the file
     * ends on a 4-byte boundary, but this is handled in write().
     */
    private static final byte[] trailer = new byte[] { (byte)0x82, 0x00,
                                                             0x09, 0x00,
                                                             0x03, 0x00,
                                                             (byte)0x82, 0x00,
                                                             0x00, 0x00,
                                                             0x00, 0x00,
                                                             0x00, 0x00,
                                                             0x00, 0x00,
                                                             0x00, 0x00 };


    /**
     * <p>Constructs a new Pocket Word Document.</p>
     *
     * <p>This new document does notcontain any information.  Document data must
     *    either be added using appropriate methods, or an existing file can be
     *    {@link #read(InputStream) read} from an <code>InputStream</code>.</p>
     *
     * @param   name    The name of the <code>PocketWordDocument</code>.
     */
    public PocketWordDocument(String name) {

        docName = trimDocumentName(name);

        preamble   = new byte[52];
        fonts      = new ArrayList<Object>();
        descriptor = new DocumentDescriptor();
        paragraphs = new ArrayList<Paragraph>();
    }


    /**
     * <p>This method reads <code>byte</code> data from the InputStream and
     *    extracts font and paragraph data from the file.</p>
     *
     * @param   docData         InputStream containing a Pocket Word data file.
     *
     * @throws  IOException     In case of any I/O errors.
     */
    public void read(InputStream docData) throws IOException {

        if (docData == null) {
            throw new IOException ("No input stream to convert");
        }

        // The preamble may become important for font declarations.
        int readValue = docData.read(preamble);
        // #i33702# check for an empty InputStream.
        if(readValue == -1) {
            System.err.println("Error:invalid input stream");
            return;
        }

        byte[] font = new byte[80];
        do {
            docData.read(font);

            String name = new String(font, 0, 64, "UTF-16LE");
            fonts.add(name.trim());

        } while (!(font[76] == 5 && font[77] == 0
                            && font[78] == 1 && font[79] == 0));

        /*
         * TODO:  The document descriptor data that follows the fonts ends with
         *        a variable section containing data for each of the paragraphs.
         *        It may be possible to use this information to calculate staring
         *        positions for each paragraph rather than iterating through the
         *        entire byte stream.
         */

        int value;
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        while ((value = docData.read()) != -1) {
            bos.write(value);
        }


        byte[] contentData = bos.toByteArray();
        int start = 0, end = 0;
        boolean sawMarker = false;

        for (int i = 0; i < contentData.length; i += 4) {
            if (contentData[i  + 2] == (byte)0xFF
                        && contentData[i + 3] == (byte)0xFF && !sawMarker)  {
                start = i - 8;
                sawMarker = true;
                continue;
            }

            if (contentData[i + 2] == (byte)0xFF
                        && contentData[i + 3] == (byte)0xFF && sawMarker) {
                end = i - 8;
                ByteArrayOutputStream paragraph = new ByteArrayOutputStream();
                paragraph.write(contentData, start, end - start);
                paragraphs.add(new Paragraph(paragraph.toByteArray()));

                // Reset the markers
                sawMarker = false;
                i -= 4;  // Skip back
            }

        }

        /*
         * Special case, the last paragraph
         * If we got here, and the marker is set then we saw the start of the
         * last paragraph, but no following paragraph
         */
        ByteArrayOutputStream paragraph = new ByteArrayOutputStream();
        if (contentData[contentData.length - 19] == 0) {
            paragraph.write(contentData, start, contentData.length - start - 20);
        }
        else {
            paragraph.write(contentData, start, contentData.length - start - 18);
        }
        paragraphs.add(new Paragraph(paragraph.toByteArray()));
    }


    /*
     * Utility method to make sure the document name is stripped of any file
     * extensions before use.
     */
    private String trimDocumentName(String name) {
        String temp = name.toLowerCase();

        if (temp.endsWith(FILE_EXTENSION)) {
            // strip the extension
            int nlen = name.length();
            int endIndex = nlen - FILE_EXTENSION.length();
            name = name.substring(0,endIndex);
        }

        return name;
    }


    /**
     * <p>Method to provide access to all of the <code>Paragraph</code> objects
     *    in the <code>Document</code>.</p>
     *
     * @return <code>Enumeration</code> over the paragraphs in the document.
     */
    public Iterator<Paragraph> getParagraphEnumeration() {
        return paragraphs.iterator();
    }


    /**
     * <p>Returns the <code>Document</code> name with no file extension.</p>
     *
     * @return  The <code>Document</code> name with no file extension.
     */
    public String getName() {
        return docName;
    }


    /**
     * <p>Returns the <code>Document</code> name with file extension.</p>
     *
     * @return  The <code>Document</code> name with file extension.
     */
    public String getFileName() {
        return new String(docName + FILE_EXTENSION);
    }


    /**
     * <p>Writes out the <code>Document</code> content to the specified
     * <code>OutputStream</code>.</p>
     *
     * <p>This method may not be thread-safe.
     * Implementations may or may not synchronize this
     * method.  User code (i.e. caller) must make sure that
     * calls to this method are thread-safe.</p>
     *
     * @param  os  <code>OutputStream</code> to write out the
     *             <code>Document</code> content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        DataOutputStream dos = new DataOutputStream(os);

        initPreamble();
        dos.write(preamble);

        loadFonts();
        for (int i = 0; i < fonts.size(); i++ ) {
            ByteArrayOutputStream fontData = (ByteArrayOutputStream)fonts.get(i);
            dos.write(fontData.toByteArray());
        }


        for (int i = 0; i < paragraphs.size(); i++) {
            Paragraph para = paragraphs.get(i);
            descriptor.addParagraph((short)para.getTextLength(), para.getLines());
        }
        dos.write(descriptor.getDescriptor());

        for (int i = 0; i < paragraphs.size(); i++ ) {
            Paragraph para = paragraphs.get(i);

            // Last paragraph has some extra data
            if (i + 1 == paragraphs.size()) {
                para.setLastParagraph(true);
            }
            dos.write(para.getParagraphData());
        }


        /*
         * Before we write out the trailer, we need to make sure that it will
         * lead to the file ending on a 4 byte boundary.
         */
        if (dos.size() % 4 == 0) {
            dos.write((byte)0x00);
            dos.write((byte)0x00);
        }

        dos.write(trailer);

        dos.flush();
        dos.close();
    }


    /**
     * <p>This method adds a new paragraph element to the document.  No string
     *    data is added to the paragraph.</p>
     *
     * <p><b>N.B.</b> The newly added paragraph becomes the current paragraph and
     *    is used as the target for all subsequent calls to addParagraphData().</p>
     *
     * @param   style       Paragraph Style object describing the formatting for
     *                      the new paragraph.  Can be null.
     * @param   listElement true if this paragraph is to be bulleted;
     *                      false otherwise.
     */
    public void addParagraph(ParaStyle style, boolean listElement)  {
        /* For the moment, only support basic text entry in a single paragraph */
        Paragraph para = new Paragraph(style);

        paragraphs.add(para);

        currentPara = para;

        if (listElement) {
            para.setBullets(true);
        }
    }


    /**
     * <p>This method adds text to the current paragraph.</p>
     *
     * <p>If no paragraphs exist within the document, it creates one.</p>
     *
     * @param   data        The string data for this segment.
     * @param   style       Text Style object describing the formatting of this
     *                      segment.  Can be null.
     */
    public void addParagraphData(String data, TextStyle style) {
        if (currentPara == null) {
            addParagraph(null, false);
        }
        currentPara.addTextSegment(data, style);
    }


    /*
     * Preamble is the portion before font specification which never
     * seems to change from one file, or one saved version, to the next.
     *
     * Bytes 18h and 19h seem to contain the number of fonts and should
     * be modified when all of the fonts have been specified.
     * These bytes are the first two on the fourth line below.
     */
    private void initPreamble() {
         preamble = new byte[] { 0x7B, 0x5C, 0x70, 0x77, 0x69, 0x15, 0x00, 0x00,
                                 0x01, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x02, 0x00, 0x2C, 0x00, 0x01, 0x00, 0x0A, 0x00,  // Bytes 3-4 Font??
                                 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Bytes 1-2  # Fonts
                                 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00 };
    }


    /*
     * This method writes the minimum font data that is used by the converter.
     * Currently, all documents convert to 10 point Courier New.  Tahoma is
     * always mentioned in Pocket Word files, however, even if it is not used.
     *
     * TODO:    Rewrite to allow for multiple fonts once font support issues
     *          have been resolved.
     */
    private void loadFonts() {
        ByteArrayOutputStream fontData = new ByteArrayOutputStream();

        try {
            fontData.write(new String("Tahoma").getBytes("UTF-16LE"));
            fontData.write(new byte[52]);       // Rest of font name?
            fontData.write(new byte[] { 0x02, 0x00, 0x01, 0x00 } );
            fontData.write(new byte[] { 0x00, 0x00, 0x01, 0x00 } );
            fontData.write(new byte[] { 0x00, 0x00, 0x00, 0x00 } );
            fontData.write(new byte[] { 0x00, 0x00, 0x00, 0x00 } );

            fonts.add(fontData);

            fontData = new ByteArrayOutputStream();

            fontData.write(new String("Courier New").getBytes("UTF-16LE"));
            fontData.write(new byte[42]);
            fontData.write(new byte[] { 0x14, 0x00, 0x04, 0x00 } );
            fontData.write(new byte[] { 0x01, 0x00, 0x00, 0x00 } );
            fontData.write(new byte[] { 0x00, 0x00, 0x15, 0x00 } );

            // Next part indicates that this is the last font
            fontData.write(new byte[] { 0x05, 0x00, 0x01, 0x00 } );

            fonts.add(fontData);
        }
        catch (IOException ioe) {
            // Shouldn't happen as this is a memory based stream
        }
    }
}
