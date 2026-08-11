/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.ParcelFileDescriptor;
import android.print.PageRange;
import android.print.PrintAttributes;
import android.print.PrintDocumentAdapter;
import android.print.PrintDocumentInfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Objects;

public class PrintAdapter extends PrintDocumentAdapter {

    private File printDocFile;
    private LOActivity mainActivity;

    PrintAdapter(LOActivity mainActivity) {
        this.mainActivity = mainActivity;
    }

    @Override
    public void onStart() {
        super.onStart();
        //Will show its own progress bar for the below task
        printDocFile = new File(mainActivity.getCacheDir(), "print.pdf");
        // Leave form fields and comments out of the printout, like the
        // browser does when printing to PDF.
        mainActivity.saveAs(printDocFile.toURI().toString(), "pdf",
                "{\"ExportFormFields\":{\"type\":\"boolean\",\"value\":\"false\"},"
                + "\"ExportNotes\":{\"type\":\"boolean\",\"value\":\"false\"}}");
    }

    @Override
    public void onLayout(PrintAttributes oldAttributes, PrintAttributes newAttributes,
                         CancellationSignal cancellationSignal, LayoutResultCallback callback,
                         Bundle extras) {
        if (cancellationSignal.isCanceled()) {
            callback.onLayoutCancelled();
        } else {
            PrintDocumentInfo.Builder builder =
                    new PrintDocumentInfo.Builder("finalPrint.pdf");
            builder.setContentType(PrintDocumentInfo.CONTENT_TYPE_DOCUMENT)
                    .setPageCount(PrintDocumentInfo.PAGE_COUNT_UNKNOWN)
                    .build();
            callback.onLayoutFinished(builder.build(),
                    !newAttributes.equals(oldAttributes));
        }
    }

    @Override
    public void onWrite(PageRange[] pages, ParcelFileDescriptor destination, CancellationSignal cancellationSignal,
                        WriteResultCallback callback) {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = new FileInputStream(printDocFile);
            out = new FileOutputStream(destination.getFileDescriptor());

            byte[] buf = new byte[16384];
            int size;

            while ((size = in.read(buf)) >= 0
                    && !cancellationSignal.isCanceled()) {
                out.write(buf, 0, size);
            }

            if (cancellationSignal.isCanceled()) {
                callback.onWriteCancelled();
            } else {
                callback.onWriteFinished(new PageRange[]{PageRange.ALL_PAGES});
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                Objects.requireNonNull(in).close();
                Objects.requireNonNull(out).close();
            } catch (IOException | NullPointerException e) {
                e.printStackTrace();
            }
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
