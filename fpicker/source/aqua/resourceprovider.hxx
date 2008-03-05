/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourceprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:40:06 $
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


#ifndef _RESOURCEPROVIDER_HXX_
#define _RESOURCEPROVIDER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#define FOLDERPICKER_TITLE            500
#define FOLDER_PICKER_DEF_DESCRIPTION 501
#define FILE_PICKER_TITLE_OPEN        502
#define FILE_PICKER_TITLE_SAVE        503
#define FILE_PICKER_FILE_TYPE         504
#define FILE_PICKER_OVERWRITE         505

//------------------------------------------------------------------------
// declarations
//------------------------------------------------------------------------

class CResourceProvider_Impl;

class CResourceProvider
{
public:
    CResourceProvider( );
    ~CResourceProvider( );

    NSString* getResString( sal_Int32 aId );

private:
    CResourceProvider_Impl* m_pImpl;
};

#endif
