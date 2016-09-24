package org.libreoffice.storage;

import android.content.Context;
import android.util.Log;

import org.libreoffice.R;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URI;
import java.net.URISyntaxException;

/**
 * File IO related methods.
 */
public class IOUtils {
    private static final int BUFFER_SIZE = 1024 * 8;
    private static final String LOGTAG = IOUtils.class.getSimpleName();

    public static File getFileFromURIString(String URIpath) throws IllegalArgumentException{
        try{
            return new File(new URI(URIpath));
        } catch (URISyntaxException e) {
            //should not happen as all URIs are system generated
            Log.wtf(LOGTAG, e.getReason());
            return null;
        }
    }

    public static boolean isInvalidFile(File f) {
        return f == null || !f.exists() || f.getTotalSpace() == 0
                || !f.canRead() || !f.canWrite();
    }

    public static int copy(InputStream input, OutputStream output) throws Exception {
        byte[] buffer = new byte[BUFFER_SIZE];

        BufferedInputStream in = new BufferedInputStream(input, BUFFER_SIZE);
        BufferedOutputStream out = new BufferedOutputStream(output, BUFFER_SIZE);

        int count = 0, n = 0;
        try {
            while ((n = in.read(buffer, 0, BUFFER_SIZE)) != -1) {
                out.write(buffer, 0, n);
                count += n;
            }
            out.flush();
        } finally {
            if (out != null) out.close();
            if (in != null) in.close();
        }

        return count;
    }

}
