package org.libreoffice;

import android.os.Environment;
import android.util.Log;

import com.ringlord.odf.Container;
import com.ringlord.odf.Entry;

import net.lingala.zip4j.core.ZipFile;
import net.lingala.zip4j.exception.ZipException;
import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.util.Zip4jConstants;

import org.libreoffice.ui.LibreOfficeUIActivity;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class Decryptor {

    public static File getDecryptedFile(File file, String password) throws Exception {
        final String tempPath = Environment.getExternalStorageDirectory()
                + "/Android/data/com.example.libreoffice/temp/";

        // destroys the old decrypted file
        new File(tempPath + "temp.file").delete();

        final Container odf = new Container(file);

        try {
            for (Entry e : odf) {
                e.setOutput(tempPath);

                if (e.isEncrypted()) {
                    try {
                        Log.i(LibreOfficeUIActivity.class.getSimpleName(), password);
                        e.data(password);
                    } catch(Exception x) {
                        x.printStackTrace();
                    }
                }
            }
        } finally {
            odf.close();
        }

        convertManifest(tempPath + "META-INF/manifest.xml");
        compress(tempPath);
        deleteRecursive(tempPath);

        return new File(tempPath + "temp.file");
    }

    /**
     * Checks for encryption by decompressing the file in an external directory and checks the
     * manifest.
     * Returns true in case it is, else it will delete all extracted files and return false.
     */
    public static boolean isEncrypted(File file) throws IOException {
        final String tempPath = Environment.getExternalStorageDirectory()
                + "/Android/data/com.example.libreoffice/temp/";

        try {
            decompress(file, tempPath);
        } catch (ZipException e) {
            e.printStackTrace();
        }

        BufferedReader br = new BufferedReader(new FileReader(tempPath + "META-INF/manifest.xml"));

        String line;
        while ((line = br.readLine()) != null) {
            if (line.contains("manifest:encryption-data")) {
                return true;
            }
        }

        deleteRecursive(tempPath);
        return false;
    }

    /**
     * Removes all files in the cache directory, except the new decrypted odf which is removed in
     * onCreate separately. This is mainly used to remove files after the decryption.
     */
    public static void deleteRecursive(String path) {
        File file = new File(path);

        if (file.isDirectory())
            for (File child : file.listFiles())
                if (!child.getName().equals("temp.file")) {
                    deleteRecursive(child.getAbsolutePath());
                }

        file.delete();
    }

    private static void decompress(File file, String destination) throws ZipException {
        ZipFile zipFile = new ZipFile(file.getAbsolutePath());

        // Makes sure the path to the cache exists
        File outputDirectory = new File(destination);
        outputDirectory.mkdirs();

        zipFile.extractAll(destination);
    }

    private static void compress(String path) throws ZipException {
        ZipFile zipFile = new ZipFile(Environment.getExternalStorageDirectory()
                + "/Android/data/com.example.libreoffice/temp/temp.file");

        ZipParameters zipParameters = new ZipParameters();
        zipParameters.setCompressionMethod(Zip4jConstants.COMP_DEFLATE);
        zipParameters.setCompressionLevel(Zip4jConstants.DEFLATE_LEVEL_NORMAL);
        zipParameters.setIncludeRootFolder(false);

        zipFile.addFolder(path, zipParameters);
    }

    /**
     * Removes the lines from the manifest which suggest that the odf was encrypted by copying
     * in another file every line which doesn't start with 2 or more whitespaces and then it deletes
     * the old file and renames the new one accordingly.
     */
    private static void convertManifest(String path) throws IOException {
        File input = new File(path); // manifest.xml
        File output = new File(path + ".new"); // manifest.xml.new

        BufferedReader br = new BufferedReader(new FileReader(input));
        BufferedWriter bw = new BufferedWriter(new FileWriter(output, true));

        String line;
        while ((line = br.readLine()) != null) {
            if (!line.startsWith("  ")) {
                bw.write(line);
                bw.newLine();
            }
        }

        bw.close();

        input.delete();
        output.renameTo(new File(path));
    }
}
