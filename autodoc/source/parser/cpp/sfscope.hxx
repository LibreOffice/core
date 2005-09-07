/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfscope.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:32:55 $
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

#ifndef ADC_CPP_SFSCOPE_HXX
#define ADC_CPP_SFSCOPE_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

/** Implementation struct for cpp::ContextForAry.
*/
struct ContextForAry::S_FileScopeInfo
{
    udmstri             sCurFileName;
    ary::cpp::ProjectGroup *
                        pCurProject;
    ary::cpp::FileGroup *
                        pCurFile;
    uintt               nLineCount;
    Dyn<StringVector>   pCurTemplateParameters;

                        S_FileScopeInfo();
};


inline
ContextForAry::
S_FileScopeInfo::S_FileScopeInfo()
    :   // sCurFileName,
        pCurProject(0),
        pCurFile(0),
        nLineCount(0)
        // pCurTemplateParameters
{
}

}   // namespace cpp


#endif

