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


/**
 *
 * @author ll93751
 */
public class PostscriptCreator extends EnhancedComplexTestCase
{

    // @Override
    public String[] getTestMethodNames()
    {
        return new String[]{"DocumentToPostscript"};
    }

    /**
     * test function.
     */
    public void DocumentToPostscript()
    {
        GlobalLogWriter.set(log);
        ParameterHelper aParam = new ParameterHelper(param);

        param.put(util.PropertyName.OFFICE_CLOSE_TIME_OUT, 2000);
        // run through all documents found in Inputpath
        foreachDocumentinInputPath(aParam);
    }


    public void checkOneFile(String _sDocumentName, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
        GlobalLogWriter.println("  Document: " + _sDocumentName);
        GlobalLogWriter.println("   results: " + _sResult);
        IOffice aOffice = new Office(_aParams, _sResult);

        PerformanceContainer a = new PerformanceContainer();
        a.startTime(PerformanceContainer.AllTime);

        // _aParams.getTestParameters().put(util.PropertyName.DEBUG_IS_ACTIVE, Boolean.TRUE);
        a.startTime(PerformanceContainer.OfficeStart);
        aOffice.start();
        a.stopTime(PerformanceContainer.OfficeStart);

        // _aParams.getTestParameters().put(util.PropertyName.DEBUG_IS_ACTIVE, Boolean.FALSE);

        // This force an error! _sDocumentName = helper.StringHelper.doubleQuote(_sDocumentName);
        try
        {
            a.startTime(PerformanceContainer.Load);
            aOffice.load(_sDocumentName);
            a.stopTime(PerformanceContainer.Load);

            a.startTime(PerformanceContainer.Print);
            aOffice.storeAsPostscript();
            a.stopTime(PerformanceContainer.Print);
        }
        finally
        {
            a.startTime(PerformanceContainer.OfficeStop);
            aOffice.close();
            a.stopTime(PerformanceContainer.OfficeStop);

            a.stopTime(PerformanceContainer.AllTime);

            a.print( System.out );
        }
    }




//    public static void main(String [] _args)
//    {
//        String args[] = {
//            "-TimeOut", "3600000",
//            "-tb", "java_complex",
//            "-o", "graphical.PostscriptCreator",
////            "-DOC_COMPARATOR_INPUT_PATH", "D:\\temp\\input",
////            "-DOC_COMPARATOR_OUTPUT_PATH", "D:\\temp\\output",
////            "-DOC_COMPARATOR_REFERENCE_PATH", "D:\\temp\\output\\ref",
////            "-DOC_COMPARATOR_PRINT_MAX_PAGE", "9999",
////            "-DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION", "180",
////            "-DOC_COMPARATOR_HTML_OUTPUT_PREFIX", "http://so-gfxcmp-lin.germany.sun.com/gfxcmp_ui/cw.php?inifile=",
//////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "PDF",      /* default: "OOo" */
//////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "msoffice", /* default: "OOo" */
//////            "-OFFICE_VIEWABLE", "false",
////            "-AppExecutionCommand", "\"C:/home/ll93751/staroffice9_DEV300_m25/Sun/StarOffice 9/program/soffice.exe\"  -norestore -nocrashreport -accept=pipe,name=ll93751;urp;",
//////            "-NoOffice"
//            "-DOC_COMPARATOR_PRINT_MAX_PAGE","9999",
//            "-DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION","180",
//            "-DOC_COMPARATOR_HTML_OUTPUT_PREFIX","http://so-gfxcmp.germany.sun.com/gfxcmp_ui/cw.php?inifile=",
//            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE","OOo",
//            "-DOC_COMPARATOR_DB_INFO_STRING","p:DEV300_m18,c:,d:LLA_test,src:DEV300_m18,dest:,doc:LLA_test,id:34715,distinct:2008-06-27_13-39-09_d6f22d4c-958d-10",
//            "-DISTINCT","2008-06-27_13-39-09_d6f22d4c-958d-10",
//            "-TEMPPATH","//so-gfxcmp-lin/gfxcmp-data/wntmsci/temp/2008-06-27_13-39-09_d6f22d4c-958d-10/34715",
////            "ConnectionString","socket,host=localhost,port=8101",
//            "-OFFICE_VIEWABLE","true",
//            "-wntmsci.DOC_COMPARATOR_INPUT_PATH","\\\\so-gfxcmp-lin\\doc-pool\\LLA_test\\issue_79214.odb",
//            "-wntmsci.DOC_COMPARATOR_OUTPUT_PATH","\\\\so-gfxcmp-lin\\gfxcmp-data\\wntmsci\\convwatch-output\\LLA_test\\DEV300_m11",
//            "-wntmsci.AppExecutionCommand","\"C:\\gfxcmp\\programs\\staroffice8_DEV300_m11\\Sun\\StarOffice 9\\program\\soffice.exe\"  -norestore -nocrashreport -accept=pipe,name=ll93751;urp;",
//            "-wntmsci.AppKillCommand","\"C:\\bin\\pskill.exe soffice.bin;C:\\bin\\pskill.exe winword;C:\\bin\\pskill.exe excel\"",
//
//        };
//
//        org.openoffice.Runner.main(args);
//    }

}
