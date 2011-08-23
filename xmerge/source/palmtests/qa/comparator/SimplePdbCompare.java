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

/*
 * SimplePdbCompare.java
 *
 * Created on September 21, 2001, 10:23 AM
 */

/**
 *
 * @author  mh101528
 * @version 
 */
public final class SimplePdbCompare {

    /** Creates new SimplePdbCompare */
    public SimplePdbCompare() {
    }

    /**
    * @param args the command line arguments
    */
    public static void main (String args[]) 
    {
        SimplePdbCompare comparator = new SimplePdbCompare();
        if (comparator.comparePDB(args[0], args[1]))
            System.exit(2);
        else
            System.exit(3);
    }
    
    public boolean  comparePDB(String pdbname1, String pdbname2)
    {
        PalmDB pdb1=null, pdb2=null;
        PDBDecoder decoder = new PDBDecoder();
        try 
        {
            pdb1 = decoder.parse(pdbname1);
        } 
        catch (Exception e) 
        {
            System.out.println("Could not parse PDB " + pdbname1);
            return false;
        }

        try 
        {
            pdb2 = decoder.parse(pdbname2);
        }
        catch (Exception e)
        {
            System.out.println("Could not parse PDB " + pdbname2);
            return false;
        }

        if (pdb1.equals(pdb2)) 
        {
            //writeToLog("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are equal");
            System.out.println("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are equal");
            return true;
        } 
        else 
        {
            //writeToLog("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are not equal");
            System.out.println("PDB " + pdbname1 + "  and PDB " + pdbname2 + " are not equal");
            return false;
        }
    } 
}
