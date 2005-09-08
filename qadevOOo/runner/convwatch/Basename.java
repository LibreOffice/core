/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Basename.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:08:54 $
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
