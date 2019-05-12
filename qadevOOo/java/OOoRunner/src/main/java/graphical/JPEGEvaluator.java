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
