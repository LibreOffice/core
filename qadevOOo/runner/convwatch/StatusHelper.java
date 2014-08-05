/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package convwatch;

public class StatusHelper
{
    final static public int DIFF_NOT_REALLY_INITIALISED =            1;
    final static public int DIFF_NO_DIFFERENCES =                    2;
    final static public int DIFF_DIFFERENCES_FOUND =                 3;
    final static public int DIFF_AFTER_MOVE_DONE_NO_PROBLEMS =       4;
    final static public int DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND = 5;

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

}
