/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

import java.io.*;
import java.util.*;

public class pdbcomparison
{

  private String LOGTAG  ="LOGFILE";
  private String OUTTAG  ="OUTFILE";
  private String LISTTAG ="LISTFILE";
  private String PDBTAG1 ="PDBNAME1";
  private String PDBTAG2 ="PDBNAME2";

  private String OUTFILE="pdbcomparison.out";
  private String LOGFILE="pdbcomparison.log";

  private String pdbarr1[];
  private String pdbarr2[];
 

   /**
   * Default Constructor
   *
   *  @param
   *  @return
   *
   */
   public void pdbcomparison()
   {
   }

   /**
   * Prints the command line arguments for this class
   *
   * @param
   *
   * @return void
   *
   */
   public void usage()
   {
      String str = new String();
      str += "********************************************************\n";
      str += " java pdbcomparison.java <propFile> \n";
      str += "   where propFile is name of Property File...\n";
      str += "********************************************************\n";

      System.out.println(str);
     
   }

   /**
   * This method, read the Property file and validates the
   * entries in that file, and accordingly sets the log file
   * output file and updates the array pdbarr1 and pdbarr2 with
   * list of pdb's to be compared.
   *
   * @param propFile Property filename which list the log/outputfile/list/pdb 
   *                 names
   * @return
   *
   */
   public void parsePropertyFile(String propFile)
   {
     Properties defaultProps = new Properties();

     try {
       FileInputStream in = new FileInputStream(propFile);
       defaultProps.load(in);
       in.close();
     } catch (IOException e) {
       System.out.println("Could not open Property File " + propFile);
       return;
     }
     

     String logFile  = defaultProps.getProperty(this.LOGTAG);
     String outFile  = defaultProps.getProperty(this.OUTTAG);
     String listFile = defaultProps.getProperty(this.LISTTAG);
     String pdbname1 = defaultProps.getProperty(this.PDBTAG1);
     String pdbname2 = defaultProps.getProperty(this.PDBTAG2);

    // validate all command line arguments
    if ((listFile == null) && ((pdbname1 == null) || (pdbname2 == null)))
    {
       System.out.println("Missing listFile or missing pdb filenames in Property file " + propFile);
       return;
    }

    if (logFile == null || logFile.length() == 0)
       logFile = this.LOGFILE; 

    if (outFile == null || outFile.length() == 0)
       outFile = this.LOGFILE; 


     // validate log and output files
     if (! validateAndCreateFile(logFile)) return;
     if (! validateAndCreateFile(outFile)) return;
     LOGFILE = logFile;
     OUTFILE = outFile;

     System.out.println("Output is written to log file... " + LOGFILE); 
     if (listFile != null)
     {
       if (! checkFile(listFile)) return;
       populatePDBArray(listFile);
     } else {
       if (! checkFile(pdbname1)) return;
       if (! checkFile(pdbname2)) return;
       populatePDBArray(pdbname1, pdbname2);
     }
   }

   /**
   * This method validates if the file passed exists.
   * If it does , then it is moved to <filename>.bak and then creates a newFile.
   * Also validates permissions to create.
   *
   *  @param  filename  name of file to be created
   *  @return true, if file could be created 
   *          false, if could not.
   *
   */
   private boolean validateAndCreateFile (String filename)
   {
     if (filename == null) return false;

     File f = null;
     try {
       f = new File(filename);
     } catch (NullPointerException e) {
       System.out.println("Could not create a File object for file " + filename);
       return false;
     }

     if (f.exists())
     {
       String newFile = filename + ".bak";
       File newF=null;
       try {
         newF = new File(newFile);
       } catch (Exception ex) {
         System.out.println("Could not get File Object instance for " + newFile);
         return false;
       }

       if (newF.exists()) 
       {
         try {
           newF.delete();
         } catch ( SecurityException se) {
           System.out.println("Could not get delete " + newFile);
           return false;
         }
       }

       try {
         if (! f.renameTo(newF))
         {
             System.out.println("Could not rename " + filename + "  to " + newFile );
             return false;
         }
       } catch  (SecurityException s) {
             System.out.println("SecurityException: " + s.toString());
             return false;
       } catch  (NullPointerException n) {
             System.out.println("NullPointerException: " + n.toString());
             return false;
       }
     } else {
       try {
        if (! f.createNewFile())
        {
           System.out.println("Could not create " + filename + " Check permissions..");
           return false;
        }
       } catch (IOException e) {
           System.out.println("IOException: " + e.toString());
           return false;
       } catch  (SecurityException s) {
           System.out.println("SecuriityException: " + s.toString() );
           return false;
       }

     }
  
     return true;
 
   }

   /**
   * This method validates if the file exists and is readable
   *
   *  @param  filename  name of file to be created
   *  @return true, if file exists and is readable
   *          false, if not.
   *
   */
   private boolean  checkFile(String filename)
   {
     if (filename == null) return false;

     File f = null;
     try {
       f = new File(filename);
     } catch (NullPointerException e) {
       System.out.println("Could not create a File object for file " + filename);
       return false;
     }

     if (! f.exists())
     {
       System.out.println("File " + filename + " does not exist... ");
       return false;
     }

     if (! f.canRead())
     {
       System.out.println("Cannot read file " + filename);
       return false;
     }
  
     return true;
 
   }

   /**
   * This method populates the pdb arrays with the names of the pdbs to 
   * compare. Ths listFile lists a series of entries, wherein each
   * line indicates the PDB names to be compared.
   * <pdbname1>=<pdbname2>
   *   
   *  @param  listFile  name of the listfile
   *  @return 
   *
   */
   private void  populatePDBArray(String listFile)
   {
    // open ListFile and populate the PDB list to be compared
    if (listFile != null) 
    {
        Properties listProps = new Properties();
        try {
        FileInputStream in = new FileInputStream(listFile);
        listProps.load(in);
        in.close();
        } catch (IOException ex) {
         System.out.println("Could not open List File " + listFile);
         return;
        }
        
        pdbarr1 = new String[listProps.size()];
        pdbarr2 = new String[listProps.size()];
        Enumeration e = listProps.keys();
        int j=0;
        while (e.hasMoreElements())
        {
            pdbarr1[j] = (String)e.nextElement();
            pdbarr2[j] = listProps.getProperty(pdbarr1[j]);   
                    j++;
        }

        }
   }

   /**
   * This method populates the pdb arrays with the names of the pdbs to 
   * compare.  
   * 
   *  @param  pdbname1 Name of 2nd PDB file to be compared
   *  @param  pdbname2 Name of 2nd PDB file to be compared
   *  @return 
   *
   */
   private void  populatePDBArray(String pdbname1, String pdbname2)
   {
      if (pdbname1 == null) return;
      if (pdbname2 == null) return;

      if ((pdbname1 != null) && (pdbname2 != null)) 
      {
     pdbarr1 = new String[1];
     pdbarr2 = new String[1];

     pdbarr1[0] = pdbname1;
     pdbarr2[0] = pdbname2;
      }
   }

   /**
   * This method populates the pdb arrays with the names of the pdbs to 
   * compare.  
   * 
   *  @param  arrayno  Array number which corresponds to the pdb array 
   *                   containing  list of pdbs 
   *                   If 1 then send pdbarr1, if 2 send pdbarr2 else null
   *
   *  @return PDB string array containing list of PDB's
   *
   */
   private String[]  getPDBArray(int arrayno)
   {
    if (arrayno == 1) return pdbarr1;
    if (arrayno == 2) return pdbarr2;

    return null;
   }

   /**
   * This method comares 2 PDB's and returns true if comparison is equal.
   * It uses the PDB Decoder class to decode to a PDB structure and then
   * does record comparison
   *
   * @param  pdbname1 Name of one  PDB file  to be compared
   * @param  pdbname2 Name of other  PDB file  to be compared
   *
   * @return returns true if both PDB's are equal else returns false
   *
   */
   private boolean  comparePDB(String pdbname1, String pdbname2)
   {
       PalmDB pdb1=null, pdb2=null;
       PDBDecoder decoder = new PDBDecoder();
       try {
         pdb1 = decoder.parse(pdbname1);
       } catch (Exception e) {
         System.out.println("Could not parse PDB " + pdbname1);
         return false;
       }

       try {
         pdb2 = decoder.parse(pdbname2);
       } catch (Exception e) {
         System.out.println("Could not parse PDB " + pdbname2);
         return false;
       }

       if (pdb1.equals(pdb2)) {
        writeToLog("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are equal");
       
        return true;
       } else {
        writeToLog("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are not equal");
        return false;
       }
   } 



   /**
   *  Write message to LOGFILE
   *
   *  @param msg Message to be written to log file
   *  @return 
   *
   */
   private void writeToLog(String msg)
   {
     if (msg == null) return;

     // Get Output Stream from Log file
     RandomAccessFile raf=null;
     try {
      raf = new RandomAccessFile(LOGFILE, "rw");
     } catch (Exception e) {
      System.out.println ("Could not open file " + LOGFILE);
      return;
     }

     try {
           long len = raf.length();
           raf.seek(len);
           raf.write(msg.getBytes());
           raf.write("\n".getBytes());
      } catch (IOException e) {
         System.out.println("ERROR: Could not write to File " + LOGFILE);
         return;
      }
   } 

   /**
   *  Write status of comparison  to OUTFILE
   *
   *  @param status Indicates whether comparsion of PDB's PASSED or FAILED
   *  @param pdbname1 file name of pdb which was compared.
   *  @param pdbname2 file name of pdb which was compared.
   *
   *  @return 
   *
   */
   private void writeToOutputFile(String status, String pdbname1, String pdbname2)
   {
     if (status == null) return;
     if (pdbname1 == null) return;
     if (pdbname2 == null) return;

     String msg = pdbname1 + "=" + pdbname2 + ":" + status;

     // Get Output Stream from Log file
     RandomAccessFile raf=null;
     try {
      raf = new RandomAccessFile(OUTFILE, "rw");
     } catch (Exception e) {
      System.out.println ("Could not open file " + OUTFILE);
      return;
     }

     try {
           long len = raf.length();
           raf.seek(len);
     
           raf.write(msg.getBytes());
           raf.write("\n".getBytes());
      } catch (IOException e) {
         System.out.println("ERROR: Could not write to File " + OUTFILE);
         return;
      }

     try {
       raf.close();
     } catch (Exception e) {
       System.out.println("ERROR: Could not close File " + OUTFILE);
       return;
     }

   } 



   /**
   *  Main starting block of execution
   *
   *  @param command line args captured in an array of Strings
   *  @return 
   *
   */
   public static void main(String args[])
   {

     Date startTime = new Date();
     pdbcomparison pdbcmp = new pdbcomparison();
     int nargs = args.length;
     int status=0;

     if (nargs != 1) 
     {  
         System.out.println("Incorrect no. of arguments passed...");
         pdbcmp.usage();
         System.exit(-1);
   
     }

     String propFile = args[0];
 
     File f=null;
     try {
       f = new File(propFile);
     } catch (Exception e) {
       System.out.println("Exception: Could not open file " + propFile);
       System.exit(-1);
     }
     
     if (! f.canRead()) {
       System.out.println("Exception: " + propFile + " is not a file ");
       System.exit(-1);
     }

     if (! f.canRead()) {
       System.out.println("Exception: Cannot open file for reading. Please check permissions ");
       System.exit(-1);
     }

     // parse Property file 
     pdbcmp.parsePropertyFile(propFile);

     String pdbarr1[] = pdbcmp.getPDBArray(1);
     String pdbarr2[] = pdbcmp.getPDBArray(2);
     if ( (pdbarr1 == null) || 
          (pdbarr2 == null) ||
          (pdbarr1.length == 0) || 
          (pdbarr1.length == 0))
     { 
       System.out.println("pdbArray is empty. No PDBS to compare... \n");
       System.exit(-1);
     }


     pdbcmp.writeToLog("************** Start *****************");
     pdbcmp.writeToLog("PDB Comparison: start time " + startTime);
     for (int i=0; i<pdbarr1.length; i++)
     {
       Date pdb_startTime = new Date();
       pdbcmp.writeToLog("\n");
       pdbcmp.writeToLog("start time " + pdb_startTime);
       boolean val = pdbcmp.comparePDB(pdbarr1[i], pdbarr2[i]);
       Date pdb_endTime = new Date();
       pdbcmp.writeToLog("end time " + pdb_endTime);

       if (val) {
        pdbcmp.writeToOutputFile("PASSED", pdbarr1[i], pdbarr2[i]);
        status=0;
       } else {
        pdbcmp.writeToOutputFile("FAILED", pdbarr1[i], pdbarr2[i]);
        status=-1;
       }
     }

     Date endTime = new Date();
     pdbcmp.writeToLog("PDB Comparison: end time " + endTime);
     pdbcmp.writeToLog("************** End *****************n");
     pdbcmp.writeToLog("\n");
   
     System.exit(status);
   }
}
