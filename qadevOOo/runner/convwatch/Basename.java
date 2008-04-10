/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Basename.java,v $
 * $Revision: 1.4 $
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
import convwatch.FileHelper;

// -----------------------------------------------------------------------------
// Helper stuff to
// abstract class Name
// {
//     abstract public String getName();
// }
// class PRN extends Name
// {
//     public String getName() {return "-ref";}
// }
// class PS extends Name
// {
//     public String getName() {return "-test";}
// }
//
// public class Basename /* extends *//* implements */ {
//     String fs;
//     Basename()
//     {
//         fs = System.getProperty("file.separator");
//     }
//     String getJPEGName(String _sOutputPath, String _sBasename, String _sGS_PageOutput, Name _aPostNameAdd)
//         {
//             String sBaseNameNoSuffix = FileHelper.getNameNoSuffix(_sBasename);
//             String sJPEGName = _sOutputPath + fs + sBaseNameNoSuffix + "-" + _sGS_PageOutput + _aPostNameAdd.getName() + ".jpg";
//             return sJPEGName;
//         }
//
//     public static void main( String[] argv ) {
//
//         }
// }
