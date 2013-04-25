/**
 * Description: Global package for file operations.
 *
 * @ Author        Create/Modi     Note
 * Xiaofeng Xie    Jun 15, 2002
 *
 * @version 1.0
 * @Since MAOS1.0
 */


package net.adaptivebox.global;

import java.io.*;
import java.util.*;

public class GlobalFile {

// used by the createTempDir to give an index of temp number.
    private static int counter = -1;

/**
  * Create a temp directory in the given directory.
  * @param      prefix      the prefix for the directory.
  * @param      directory   the directory that the temp dirctory placed.
  * @return  If a temp directory is created, return a File Object, else
  * return null.
  */
    public static File createTempDir(String prefix, String directory)
            throws IOException {
        File f = null;
        String tempDir;
        boolean isCreated = false;
        do {
            if (counter == -1) {
                counter = new Random().nextInt() & 0xffff;
            }
            counter++;
            if (prefix == null)
                throw new NullPointerException();
            if (prefix.length() < 3)
                throw new IllegalArgumentException("Prefix string too short");
            if (directory == null) {
                tempDir = prefix + counter;
            } else {
                tempDir = getFileLocation(directory, prefix + counter);
            }
            f = new File(tempDir);
            isCreated = f.mkdir();
        } while (!isCreated);
        return f;
    }

/**
  * Add the given text string to the end of a given file.
  * @param      inStr       The string to be added.
  * @param      fileStr     the name of the file to be added.
  */
  public static void addStringToFile(String inStr, String fileStr) throws Exception {

    RandomAccessFile raFile = new RandomAccessFile(fileStr,"rw");
    raFile.seek(raFile.length());
    raFile.writeBytes(inStr);
    raFile.close();


//    String oldFileStr = getStringFromFile(fileStr);
//    String newFileStr = inStr;
//    if (oldFileStr != null) {
//      newFileStr = oldFileStr+inStr;
//    }
//    saveStringToFile(newFileStr,fileStr);

  }

  public static Object loadObjectFromFile(String fileName) throws Exception {
    FileInputStream fis = new FileInputStream(fileName);
    ObjectInputStream ois = new ObjectInputStream(fis);
    Object obj = ois.readObject();
    ois.close();
    return obj;
  }

  public static void saveObjectToFile(String fileName, Object obj) throws Exception {
    FileOutputStream ostream = new FileOutputStream(fileName);
    ObjectOutputStream p = new ObjectOutputStream(ostream);
    p.writeObject(obj);
    p.flush();
    ostream.close();
  }

/**
  * Save the given text string to a given file.
  * @param      inStr       The string to be saved.
  * @param      fileStr     the name of the file to be saved.
  */
  public static void saveStringToFile(String inStr, String fileStr) throws Exception{
    new File(new File(fileStr).getParent()).mkdirs();
    FileOutputStream pspOutputStream = new FileOutputStream(new File(fileStr));
    pspOutputStream.write(inStr.getBytes());
    pspOutputStream.close();
  }

/**
  * Load text string from a given file.
  * @param      fileStr     the name of the file to be loaded.
  * @return  A text string that is the content of the file. if the given file is
  * not exist, then return null.
  */
  public static String getStringFromFile(String fileStr) throws Exception {
    String getStr = null;
    FileInputStream pspInputStream = new FileInputStream(fileStr);
    byte[] pspFileBuffer = new byte[pspInputStream.available()];
    pspInputStream.read(pspFileBuffer);
    pspInputStream.close();
    getStr = new String(pspFileBuffer);
    return(getStr);
  }

/**
  * Load curve data from a specified file.
  * @param      fileStr     the name of the file to be loaded.
  * @return  An ArrayList that include the curve data.
  */
  public static ArrayList<ArrayList<Double>> getCurveDataFromFile(String fileName) {
    File file = new File(fileName);
    if(!file.exists()){
      //showMessage();
      return null;
    }
    //open data file
    FileInputStream inStream = null;
    BufferedReader inReader = null;
    try{
      inStream = new FileInputStream(file);
      inReader = new BufferedReader(new InputStreamReader(inStream));
    }catch(Exception e){
      //showMessage();
      return null;//Data file open error.
    }
    ArrayList<Double> xaxes = new ArrayList<Double>(1);
    ArrayList<Double> yaxes = new ArrayList<Double>(1);
    try{
      StringTokenizer st;
      String s;
      boolean start = false;
      while(inReader.ready()!=false){
        st = new StringTokenizer(inReader.readLine());
        over:{
        while(!st.hasMoreTokens()){//Justify blank lines.
          if(inReader.ready()!=false){
            st = new StringTokenizer(inReader.readLine());
          }else
            break over;
          }
          s = st.nextToken();
          if((!start)&&(!s.startsWith("@")))
            break over;
          if(!start){
            start = true;
            break over;
          }
          if(s.startsWith("#")||s.startsWith("$")||s.startsWith("/")) break over;//Justify comment line.
          Double xaxis = null;
          Double yaxis = null;
          try{
            xaxis = Double.valueOf(s);
            xaxes.add(xaxis);
          }catch(NumberFormatException e){
            //showMessage();
            inReader.close();
            inStream.close();
            return null;//Data file data format error.
          }
          s = st.nextToken();
          try{
            yaxis = Double.valueOf(s);
            yaxes.add(yaxis);
          }catch(NumberFormatException e){
          //showMessage();
          inReader.close();
          inStream.close();
          return null;//Data file data format error.
          }
        }
      }
    }catch(Exception e){
      //showMessage();
      return null;//Uncertain data file error.
    }
    ArrayList<ArrayList<Double>> curveData = new ArrayList<ArrayList<Double>>(2);
    curveData.add(xaxes);
    curveData.add(yaxes);
    return curveData;
  }

/**
  * Get a full path of a given file name and a directory name.
  * @param      fileName     the name of the file.
  * @param      dir          the name of directory.
  * @return  The full path.
  */
  public static String getFileLocation(String dir, String fileName) {
    String realDir = dir;
    while (realDir.length()>0 && (realDir.endsWith("/")||realDir.endsWith("\\"))) {
      realDir = dir.substring(0, dir.length()-1);
    }
    return realDir+BasicTag.FILE_SEP_TAG+fileName;
  }

  public static String getFileName(String nameBody, String suffix) {
    if (suffix==null || suffix.trim().length()==0) {
      return nameBody;
    }
    String fileName = nameBody;
    if(nameBody.endsWith(".")) {
      return fileName+suffix;
    } else {
      return nameBody+"."+suffix;
    }
  }

  public static String getFileLocation(String dir, String fileNameBody, String fileNameSuffix) {
    String filename = getFileName(fileNameBody, fileNameSuffix);
    return getFileLocation(dir, filename);
  }

 public static void clear(String fileStr) throws Exception {
   File file = new File(fileStr);
   if(file.isFile()) {
     file.delete();
     return;
   }
   String[] fileNames = file.list();
   if (fileNames==null) {
     return;
   }
   for (int i=0; i<fileNames.length; i++) {
     String newFileName = GlobalFile.getFileLocation(fileStr,fileNames[i]);
     clear(newFileName);
   }
   file.delete();
 }

 public static String getFilePrefix(String fileStr) {
   int index = fileStr.lastIndexOf(BasicTag.DOT_TAG);
   if(index==-1) index = fileStr.length();
   return fileStr.substring(0, index);
 }

 public static String getFileSuffix(String fileStr) {
   String[] subNames = GlobalString.tokenize(fileStr, BasicTag.DOT_TAG);
   int subNameLen = subNames.length;
   if(subNameLen==1) return "";
   else return subNames[subNameLen-1];
 }

 public static String createTempImageFile(String origFile) throws Exception {
   return createTempImageFile(origFile, "img", ".inf");
 }

 public static String createTempImageFile(String origFile, String prefix, String suffix) throws Exception {
   File outputFile = createTempFile(prefix, suffix);
   outputFile.deleteOnExit();
   copyFile(outputFile.getAbsolutePath(), origFile);
   return outputFile.getAbsolutePath();
 }

 public static void copyFile(String imgFile, String origFile) throws Exception {
   String fileContent = GlobalFile.getStringFromFile(origFile);
   GlobalFile.saveStringToFile(fileContent, imgFile);
 }

 public static File createTempFile(String prefix, String suffix) throws Exception {
   String realSuffix = suffix;
   if (!realSuffix.startsWith(".")) realSuffix = "."+suffix;
   return File.createTempFile(prefix, realSuffix);
 }
}
