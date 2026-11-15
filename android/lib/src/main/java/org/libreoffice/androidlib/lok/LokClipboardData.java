/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib.lok;

import java.io.Serializable;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class LokClipboardData implements Serializable {
    public ArrayList<LokClipboardEntry> clipboardEntries = new ArrayList<LokClipboardEntry>();

    public String getText() {
        for (LokClipboardEntry aEntry : clipboardEntries) {
            if (aEntry.mime.startsWith("text/plain")) { // text/plain;charset=utf-8
                return new String(aEntry.data, StandardCharsets.UTF_8);
            }
        }
        return null;
    }

    public String getHtml() {
        for (LokClipboardEntry aEntry : clipboardEntries) {
            if (aEntry.mime.startsWith("text/html")){
                return new String(aEntry.data, StandardCharsets.UTF_8);
            }
        }
        return null;
    }

    public boolean writeToFile(File file) {
        try {
            FileOutputStream fileStream = new FileOutputStream(file.getAbsoluteFile());
            ObjectOutputStream oos = new ObjectOutputStream(fileStream);
            oos.writeObject(this);
            oos.close();
            fileStream.close();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    public static LokClipboardData createFromFile(File file) {
        try {
            FileInputStream fileStream = new FileInputStream(file.getAbsoluteFile());
            ObjectInputStream ois = new ObjectInputStream(fileStream);
            LokClipboardData data = (LokClipboardData)ois.readObject();
            ois.close();
            fileStream.close();
            return data;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
