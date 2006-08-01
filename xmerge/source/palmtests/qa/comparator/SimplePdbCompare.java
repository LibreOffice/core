/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimplePdbCompare.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:48:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
