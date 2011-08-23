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
#ifndef _INETIMG_HXX
#define _INETIMG_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace binfilter
{

class SvData;
class SotDataObject;
class SotDataMemberObject;

//=========================================================================

class INetImage
{
    String			aImageURL;
    String			aTargetURL;
    String			aTargetFrame;
    String			aAlternateText;
    Size			aSizePixel;

protected:
    String			CopyExchange() const;
    void			PasteExchange( const String& rString );

    void 			SetImageURL( const String& rS )		{ aImageURL = rS; }
    void 			SetTargetURL( const String& rS )	{ aTargetURL = rS; }
    void 			SetTargetFrame( const String& rS )	{ aTargetFrame = rS; }
    void 			SetAlternateText( const String& rS ){ aAlternateText = rS; }
    void			SetSizePixel( const Size& rSize ) 	{ aSizePixel = rSize; }

public:
                    INetImage(
                        const String& rImageURL,
                        const String& rTargetURL,
                        const String& rTargetFrame,
                        const String& rAlternateText,
                        const Size& rSizePixel )
                    :	aImageURL( rImageURL ),
                        aTargetURL( rTargetURL ),
                        aTargetFrame( rTargetFrame ),
                        aAlternateText( rAlternateText ),
                        aSizePixel( rSizePixel )
                    {}
                    INetImage()
                    {}

    const String&	GetImageURL() const { return aImageURL; }
    const String&	GetTargetURL() const { return aTargetURL; }
    const String&	GetTargetFrame() const { return aTargetFrame; }
    const String&	GetAlternateText() const { return aAlternateText; }
    const Size&		GetSizePixel() const { return aSizePixel; }
};

}

#endif // #ifndef _INETIMG_HXX


