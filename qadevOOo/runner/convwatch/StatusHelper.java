/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StatusHelper.java,v $
 * $Revision: 1.5 $
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
