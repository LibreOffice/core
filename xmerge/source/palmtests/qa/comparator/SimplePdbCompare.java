/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
