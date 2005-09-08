/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: twain.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:11:47 $
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

#ifndef _TWAIN_HXX
#define _TWAIN_HXX

// please add new platforms if TWAIN is available
#if defined WIN || defined WNT || defined UNX
#define TWAIN_AVAILABLE 1
#else
#undef  TWAIN_AVAILABLE
#endif

// include following only, if define is not set
#ifndef _TWAIN_HXX_CHECK

#ifndef _SV_BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif

// -----------
// - Defines -
// -----------

#define TWAIN_ERR_NO_ERROR                  0
#define TWAIN_ERR_MODULE_NOT_LOADED         2
#define TWAIN_ERR_DSMENTRY_NOT_FOUND        4
#define TWAIN_ERR_SOURCE_MANAGER_NOT_OPENED 6
#define TWAIN_ERR_SOURCE_SELECTION_DIALOG   8

// ---------
// - Twain -
// ---------

struct TwainImp;
namespace com { namespace sun { namespace star { namespace lang { struct EventObject; } } } }

class Twain
{
friend class TwainEventListener;

private:

    TwainImp*           mpImp;

    Bitmap              maBitmap;
    Link                maUpdateLink;
    USHORT              mnErrorCode;
    BOOL                mbScanning;

                        Twain();
                        Twain( const Twain& rTwain );
    const Twain&        operator=( const Twain& rTwain ) { return *this; }

protected:

    void                Disposing( const com::sun::star::lang::EventObject& rEventObject );

public:

                        Twain( const Link& rUpdateLink );
                        ~Twain();

    BOOL                SelectSource();
    BOOL                PerformTransfer();

    Bitmap              GetBitmap();

    BOOL                IsScanning() const { return mbScanning; }
    USHORT              GetErrorCode() const { return mnErrorCode; }
};

#endif // _TWAIN_HXX_CHECK
#endif // _TWAIN_HXX
