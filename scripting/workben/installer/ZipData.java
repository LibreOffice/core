package installer;

import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class ZipData
{
    private static String zipfile=null;

    public  ZipData(String file){
    zipfile=file;
    }

    public static boolean extractEntry(String entry, String destination, javax.swing.JLabel statusLabel){
    boolean status = false;
        System.err.println("Copying: "+entry);
        System.err.println("in: "+zipfile);
        System.err.println(" to: "+destination);
    if (statusLabel != null) {
        statusLabel.setText("Copying " + entry);
        System.out.println("Copying " + entry);
    }
    try{
        ZipFile zip = new ZipFile(zipfile);
        ZipEntry zentry = zip.getEntry(entry);
        InputStream is =zip.getInputStream(zentry);
        if (entry.lastIndexOf("/")!=-1){
        entry=entry.substring(entry.lastIndexOf("/")+1,entry.length());
        destination = destination.concat(entry);
        }
        else{
        destination = destination.concat(entry);
        }

        System.out.println("\n Unzipping "+zentry.getName()+" to "+destination);
        FileOutputStream fos = new FileOutputStream(destination);
        int bytesread=0,offset=0;
        byte[] bytearr = new byte[10000];
        bytesread= is.read(bytearr);
        while (bytesread!=-1){
        fos.write(bytearr, 0,bytesread);
         bytesread= is.read(bytearr);
        offset=offset+bytesread;
        }
        fos.close();
        is.close();
        status = true;

    }
    catch(Exception e){
       System.out.println("\nZip Error: File not found");
       System.out.println(e.getMessage());
       e.printStackTrace();
        status = false;
        if (statusLabel != null) {
            statusLabel.setText("Failed extracting " + entry + " , please view SFramework.log");
            System.out.println("Failed extracting " + entry + " to " + destination);
        }
    }
    return status;
    }


    public static void getContents(){
    try
        {
            ZipFile zip = new ZipFile(zipfile);

            ZipEntry entry = null;

            Object ObjArray[] =new Object[zip.size()];
            int i =0;
            for (Enumeration e = zip.entries(); e.hasMoreElements(); entry = (ZipEntry)e.nextElement())
            {
                if (entry != null){
                //System.out.println(entry.getName()+"\n");
                ObjArray[i]=entry.getName();
                i++;
                }

            }

        }
        catch (IOException e)
        {
            System.err.println(e);
        }
    }

    public static void getContents(String zipfile){
    try
        {
            ZipFile zip = new ZipFile(zipfile);

            ZipEntry entry = null;
            for (Enumeration e = zip.entries(); e.hasMoreElements(); entry = (ZipEntry)e.nextElement())
            {
                //System.out.println(entry);
            }
        }
        catch (IOException e)
        {
            System.err.println(e);
        }
    }


        public static void main(String args[])
    {
        getContents(args[0]);
    }
}
