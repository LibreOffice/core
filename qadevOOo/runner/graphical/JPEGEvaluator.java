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

package graphical;

public class JPEGEvaluator extends EnhancedComplexTestCase
{
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

        // run through all documents found in Inputpath
        foreachResultCreateHTML(aParam);
    }

    public void checkOneFile(String _sDocument, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
        String sBasename = FileHelper.getBasename(_sDocument);
        String sResultIniFile = _sDocument + ".ini";

        HTMLResult aOutputter = new HTMLResult(_sResult, sBasename + ".html" );
        aOutputter.header(_sResult);
        aOutputter.indexSection(sBasename);

        IniFile aResultIniFile = new IniFile(sResultIniFile);
        String sStatusRunThrough = aResultIniFile.getValue("global", "state");
        String sStatusMessage = aResultIniFile.getValue("global", "info");

        String sHTMLFile = _aParams.getHTMLPrefix(); // "http://so-gfxcmp-lin/gfxcmp_ui/cw.php?inifile=";
        sHTMLFile += _sDocument + ".ini";
        aOutputter.indexLine(sHTMLFile, sBasename, sStatusRunThrough, sStatusMessage);
        aOutputter.close();
    }
}
