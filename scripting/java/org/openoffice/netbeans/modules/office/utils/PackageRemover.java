package org.openoffice.netbeans.modules.office.utils;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;

import org.openoffice.idesupport.zip.ParcelZipper;

public class PackageRemover {
    private PackageRemover() {
    }

    public static void removeDeclaration(File source) throws IOException {
        File tmp = new File(source.getAbsolutePath() + ".tmp");

        BufferedReader in = new BufferedReader(new FileReader(source));
        BufferedWriter out = new BufferedWriter(new FileWriter(tmp));

        String line;
        while ((line = in.readLine()) != null) {
            if (line.startsWith("package") && line.indexOf(ParcelZipper.CONTENTS_DIRNAME) != -1) {
                // got package declaration, do not write it
                continue;
            }
            else {
                out.write(line, 0, line.length());
                out.newLine();
            }
        }

        while ((line = in.readLine()) != null) {
            out.write(line, 0, line.length());
            out.newLine();
        }

        in.close();
        out.close();

        if (source.delete() == false) {
            tmp.delete();
            throw new IOException("Could not overwrite " + source);
        }
        else
            tmp.renameTo(source);
    }

    public static void main(String[] args) {
        File source = new File(args[0]);

        try {
            removeDeclaration(source);
        }
        catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }
}
