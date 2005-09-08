/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetimg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:43:09 $
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
#ifndef _INETIMG_HXX
#define _INETIMG_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

class SvData;
class SotDataObject;
class SotDataMemberObject;

//=========================================================================

class INetImage
{
    String          aImageURL;
    String          aTargetURL;
    String          aTargetFrame;
    String          aAlternateText;
    Size            aSizePixel;

protected:
    String          CopyExchange() const;
    void            PasteExchange( const String& rString );

    void            SetImageURL( const String& rS )     { aImageURL = rS; }
    void            SetTargetURL( const String& rS )    { aTargetURL = rS; }
    void            SetTargetFrame( const String& rS )  { aTargetFrame = rS; }
    void            SetAlternateText( const String& rS ){ aAlternateText = rS; }
    void            SetSizePixel( const Size& rSize )   { aSizePixel = rSize; }

public:
                    INetImage(
                        const String& rImageURL,
                        const String& rTargetURL,
                        const String& rTargetFrame,
                        const String& rAlternateText,
                        const Size& rSizePixel )
                    :   aImageURL( rImageURL ),
                        aTargetURL( rTargetURL ),
                        aTargetFrame( rTargetFrame ),
                        aAlternateText( rAlternateText ),
                        aSizePixel( rSizePixel )
                    {}
                    INetImage()
                    {}

    const String&   GetImageURL() const { return aImageURL; }
    const String&   GetTargetURL() const { return aTargetURL; }
    const String&   GetTargetFrame() const { return aTargetFrame; }
    const String&   GetAlternateText() const { return aAlternateText; }
    const Size&     GetSizePixel() const { return aSizePixel; }

    // Im-/Export
    sal_Bool Write( SvStream& rOStm, ULONG nFormat ) const;
    sal_Bool Read( SvStream& rIStm, ULONG nFormat );
};

#endif // #ifndef _INETIMG_HXX


