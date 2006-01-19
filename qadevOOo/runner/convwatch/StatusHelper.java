/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatusHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 14:22:47 $
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
            GlobalLogWriter.get().println("  Original file: " + m_sOldGfx);
            GlobalLogWriter.get().println("       New file: " + m_sNewGfx);
            GlobalLogWriter.get().println("Difference file: " + m_sDiffGfx);
            if (nDiffStatus == DIFF_NOT_REALLY_INITIALISED)
            {
                GlobalLogWriter.get().println("Early problem, may be the files doesn't exist.");
            }
            else if (nDiffStatus == DIFF_NO_DIFFERENCES)
            {
                GlobalLogWriter.get().println("No differences found, ok.");
            }
            else if (nDiffStatus == DIFF_DIFFERENCES_FOUND)
            {
                GlobalLogWriter.get().println("Files differ by " + String.valueOf(nPercent) + "%");
            }
            else if (nDiffStatus == DIFF_AFTER_MOVE_DONE_NO_PROBLEMS)
            {
                GlobalLogWriter.get().println("No differences found, after move picture.");
            }
            else if (nDiffStatus == DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND)
            {
                GlobalLogWriter.get().println("A picture move is done, the files differ by " + String.valueOf(nPercent2) + " old was " + String.valueOf(nPercent) + "%");
            }
            else
            {
                GlobalLogWriter.get().println("Unknown DIFF_ values used, not handles yet.");
            }
        }





//  TODO: stream output
//     public stream& statusline(stream)
//         {
//             stream << name << "PASS" << nDiff==0?"PASS":"FAIL" << endl;
//             return stream;
//         }
}
