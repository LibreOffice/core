/*************************************************************************
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
 ************************************************************************/

#ifndef _TWAIN_HXX
#define _TWAIN_HXX

// please add new platforms if TWAIN is available
#if defined WIN || defined WNT || defined UNX
#define	TWAIN_AVAILABLE	1
#else
#undef	TWAIN_AVAILABLE
#endif

// include following only, if define is not set
#ifndef _TWAIN_HXX_CHECK

#ifndef _SV_BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif

// -----------
// - Defines -
// -----------

#define TWAIN_ERR_NO_ERROR					0
#define TWAIN_ERR_MODULE_NOT_LOADED			2
#define	TWAIN_ERR_DSMENTRY_NOT_FOUND		4
#define	TWAIN_ERR_SOURCE_MANAGER_NOT_OPENED	6
#define	TWAIN_ERR_SOURCE_SELECTION_DIALOG	8

// ---------
// - Twain -
// ---------

namespace com { namespace sun { namespace star { namespace lang { struct EventObject; } } } }

namespace binfilter
{

struct TwainImp;
class Twain
{
friend class TwainEventListener;

private:
    
    TwainImp*			mpImp;

    Bitmap				maBitmap;
    Link				maUpdateLink;
    USHORT				mnErrorCode;
    BOOL				mbScanning;

                        Twain();
                        Twain( const Twain& rTwain );
    const Twain&		operator=( const Twain& rTwain ) { return *this; }

protected:

    void				Disposing( const com::sun::star::lang::EventObject& rEventObject );

public:					
                        
                        Twain( const Link& rUpdateLink );
                        ~Twain();
                
    BOOL				SelectSource();
    BOOL				PerformTransfer();

    Bitmap				GetBitmap();
                        
    BOOL				IsScanning() const { return mbScanning; }
    USHORT				GetErrorCode() const { return mnErrorCode; }
};

}
#endif // _TWAIN_HXX_CHECK
#endif // _TWAIN_HXX
