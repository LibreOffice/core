package org.libreoffice;

import android.content.Context;
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

public class PDFDocumentAdapter extends PrintDocumentAdapter{
    Context mContext;
    String pdfFile;

    public PDFDocumentAdapter(Context mContext, String pdfFile) {
        this.mContext = mContext;
        this.pdfFile = pdfFile;
    }

    @Override
    public void onLayout(PrintAttributes oldAttributes, PrintAttributes newAttributes, CancellationSignal cancellationSignal, LayoutResultCallback callback, Bundle extras) {
        if (cancellationSignal.isCanceled()) {
            callback.onLayoutCancelled();
        }
        else {
            File f = new File(pdfFile);
            PrintDocumentInfo.Builder builder=
                    new PrintDocumentInfo.Builder(f.getName());
            builder.setContentType(PrintDocumentInfo.CONTENT_TYPE_DOCUMENT)
                    .setPageCount(PrintDocumentInfo.PAGE_COUNT_UNKNOWN)
                    .build();
            callback.onLayoutFinished(builder.build(),
                    !newAttributes.equals(oldAttributes));
        }
    }

    @Override
    public void onWrite(PageRange[] pages, ParcelFileDescriptor destination, CancellationSignal cancellationSignal, WriteResultCallback callback) {
        InputStream in=null;
        OutputStream out=null;
        try {
            File file = new File(pdfFile);
            in = new FileInputStream(file);
            out=new FileOutputStream(destination.getFileDescriptor());

            byte[] buf=new byte[in.available()];
            int size;

            while ((size=in.read(buf)) >= 0
                    && !cancellationSignal.isCanceled()) {
                out.write(buf, 0, size);
            }

            if (cancellationSignal.isCanceled()) {
                callback.onWriteCancelled();
            }
            else {
                callback.onWriteFinished(new PageRange[] { PageRange.ALL_PAGES });
            }
        }
        catch (Exception e) {
            callback.onWriteFailed(e.getMessage());
            e.printStackTrace();
        }
        finally {
            try {
                in.close();
                out.close();
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

    }
}
