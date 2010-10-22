/*
 * ************************************************************************
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
 * ***********************************************************************
 */

package graphical;

// import java.io.File;

/**
 *
 * @author ll93751
 */
public class JPEGEvaluator extends EnhancedComplexTestCase
{
    // @Override
    public String[] getTestMethodNames()
    {
        return new String[]{"EvaluateResult"};
    }

    /**
     * test function.
     */
    public void EvaluateResult()
    {
        GlobalLogWriter.set(log);
        ParameterHelper aParam = new ParameterHelper(param);

        // aParam.getTestParameters().put("current_ok_status", -1);

        // run through all documents found in Inputpath
        foreachResultCreateHTML(aParam);
    }

    public void checkOneFile(String _sDocument, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
        // throw new UnsupportedOperationException("Not supported yet.");
        // int dummy = 0;

        String sBasename = FileHelper.getBasename(_sDocument);
        String sResultIniFile = _sDocument + ".ini";
//        File aFile = new File(sResultIniFile);
//        assure("Result file doesn't exists " + sResultIniFile, aFile.exists());
//
//        int good = 0;
//        int bad = 0;
//        int ugly = 0;
//
//        IniFile aResultIniFile = new IniFile(sResultIniFile);
//        int nPages = aResultIniFile.getIntValue("global", "pages", 0);
//        for (int i=0;i<nPages;i++)
//        {
//            String sCurrentPage = "page" + String.valueOf(i + 1);
//            int nPercent = aResultIniFile.getIntValue(sCurrentPage, "percent", -1);
//            if (nPercent == 0)
//            {
//                good++;
//            }
//            else if (nPercent <= 5)
//            {
//                bad ++;
//            }
//            else
//            {
//                ugly ++;
//            }
//        }
//
//        assure("Error: document doesn't contains pages", nPages > 0);

        HTMLResult aOutputter = new HTMLResult(_sResult, sBasename + ".html" );
        aOutputter.header(_sResult);
        aOutputter.indexSection(sBasename);

        IniFile aResultIniFile = new IniFile(sResultIniFile);
        String sStatusRunThrough = aResultIniFile.getValue("global", "state");
        String sStatusMessage = aResultIniFile.getValue("global", "info");

//        // TODO: this information has to come out of the ini files
//        String sStatusRunThrough = "PASSED, ";
//        String sPassed = "OK";
//
//        String sStatusMessage = "From " + nPages + " page(s) are: ";
//        String sGood = "";
//        String sBad = "";
//        String sUgly = "";
//
//        if (good > 0)
//        {
//            sGood = " good:=" + good;
//            sStatusMessage += sGood;
//        }
//        if (bad > 0)
//        {
//            sBad = " bad:=" + bad;
//            sStatusMessage += sBad;
//        }
//       if (ugly > 0)
//        {
//            sUgly = " ugly:=" + ugly;
//            sStatusMessage += sUgly;
//        }
//
//        // Failure matrix
//        //         0     1
//        // ugly    OK    FAILED
//        // bad     OK
//        // good    OK
//
//        if (ugly > 0)
//        {
//            sPassed = "FAILED";
//        }
//        else
//        {
//            if (bad > 0)
//            {
//                sPassed = "NEED A LOOK";
//            }
//            else
//            {
//                sPassed = "OK";
//            }
//        }
//        sStatusRunThrough += sPassed;
//        aResultIniFile.insertValue("global", "state", sStatusRunThrough);
//        aResultIniFile.insertValue("global", "info", sStatusMessage);
//        aResultIniFile.close();

        String sHTMLFile = _aParams.getHTMLPrefix(); // "http://so-gfxcmp-lin/gfxcmp_ui/cw.php?inifile=";
        sHTMLFile += _sDocument + ".ini";
        aOutputter.indexLine(sHTMLFile, sBasename, sStatusRunThrough, sStatusMessage);
        aOutputter.close();
        // IniFile aIniFile = new IniFile(_sDocument);
        // aIniFile.

    }


}
