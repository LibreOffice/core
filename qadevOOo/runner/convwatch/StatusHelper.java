/*************************************************************************
 *
 *  $RCSfile: StatusHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:25:39 $
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

package convwatch;

import java.io.File;
import java.io.FileWriter;

public class StatusHelper
{
    final static public int DIFF_NOT_REALLY_INITIALISED =            1;
    final static public int DIFF_NO_DIFFERENCES =                    2;
    final static public int DIFF_DIFFERENCES_FOUND =                 3;
    final static public int DIFF_AFTER_MOVE_DONE_NO_PROBLEMS =       4;
    final static public int DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND = 5;

    // public String m_sOutputPath;
    public String m_sMainName;

    public String m_sOldGfx;
    public String m_sNewGfx;
    public String m_sDiffGfx;
    public int nDiffStatus = DIFF_NOT_REALLY_INITIALISED;
    public int nPercent = 0;

    // BorderMove Values
    public int nPercent2 = 0;
    public String m_sOld_BM_Gfx;
    public String m_sNew_BM_Gfx;
    public String m_sDiff_BM_Gfx;

    public StatusHelper(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
        {
            m_sOldGfx = _sOldGfx;
            m_sNewGfx = _sNewGfx;
            m_sDiffGfx = _sDiffGfx;
        }
    public void setFilesForBorderMove(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
        {
            m_sOld_BM_Gfx = _sOldGfx;
            m_sNew_BM_Gfx = _sNewGfx;
            m_sDiff_BM_Gfx = _sDiffGfx;

        }

    public void printStatus()
        {
            System.out.println("  Original file: " + m_sOldGfx);
            System.out.println("       New file: " + m_sNewGfx);
            System.out.println("Difference file: " + m_sDiffGfx);
            if (nDiffStatus == DIFF_NOT_REALLY_INITIALISED)
            {
                System.out.println("Early problem, may be the files doesn't exist.");
            }
            else if (nDiffStatus == DIFF_NO_DIFFERENCES)
            {
                System.out.println("No differences found, ok.");
            }
            else if (nDiffStatus == DIFF_DIFFERENCES_FOUND)
            {
                System.out.println("Files differ by " + String.valueOf(nPercent) + "%");
            }
            else if (nDiffStatus == DIFF_AFTER_MOVE_DONE_NO_PROBLEMS)
            {
                System.out.println("No differences found, after move picture.");
            }
            else if (nDiffStatus == DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND)
            {
                System.out.println("A picture move is done, the files differ by " + String.valueOf(nPercent2) + " old was " + String.valueOf(nPercent) + "%");
            }
            else
            {
                System.out.println("Unknown DIFF_ values used, not handles yet.");
            }
        }





//  TODO: stream output
//     public stream& statusline(stream)
//         {
//             stream << name << "PASS" << nDiff==0?"PASS":"FAIL" << endl;
//             return stream;
//         }
}
