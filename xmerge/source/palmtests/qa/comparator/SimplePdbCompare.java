/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
