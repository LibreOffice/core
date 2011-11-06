/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _INETIMG_HXX
#define _INETIMG_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>

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
    sal_Bool Write( SvStream& rOStm, sal_uLong nFormat ) const;
    sal_Bool Read( SvStream& rIStm, sal_uLong nFormat );
};

#endif // #ifndef _INETIMG_HXX


