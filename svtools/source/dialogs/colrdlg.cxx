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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#ifndef GCC
#endif

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/colrdlg.hxx>

using rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;

// ---------------
// - ColorDialog -
// ---------------

SvColorDialog::SvColorDialog( Window* pWindow )
: mpParent( pWindow )
, meMode( svtools::ColorPickerMode_SELECT )
{
}

// -----------------------------------------------------------------------

void SvColorDialog::SetColor( const Color& rColor )
{
    maColor = rColor;
}

// -----------------------------------------------------------------------

const Color& SvColorDialog::GetColor() const
{
    return maColor;
}

// -----------------------------------------------------------------------

void SvColorDialog::SetMode( sal_Int16 eMode )
{
    meMode = eMode;
}

// -----------------------------------------------------------------------

short SvColorDialog::Execute()
{
    short ret = 0;
    try
    {
        const OUString sColor( RTL_CONSTASCII_USTRINGPARAM( "Color" ) );
        Reference< XMultiServiceFactory > xSMGR( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );

        Reference< com::sun::star::awt::XWindow > xParent( VCLUnoHelper::GetInterface( mpParent ) );

        Sequence< Any > args(1);
        args[0] = Any( xParent );

        Reference< XExecutableDialog > xDialog( xSMGR->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.cui.ColorPicker"), args), UNO_QUERY_THROW );
        Reference< XPropertyAccess > xPropertyAccess( xDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props( 2 );
        props[0].Name = sColor;
        props[0].Value <<= (sal_Int32) maColor.GetColor();
        props[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Mode" ) );
        props[1].Value <<= (sal_Int16) meMode;

        xPropertyAccess->setPropertyValues( props );

        ret = xDialog->execute();

        if( ret )
        {
            props = xPropertyAccess->getPropertyValues();
            for( sal_Int32 n = 0; n < props.getLength(); n++ )
            {
                if( props[n].Name.equals( sColor ) )
                {
                    sal_Int32 nColor = 0;
                    if( props[n].Value >>= nColor )
                    {
                        maColor.SetColor( nColor );
                    }

                }
            }
        }
    }
    catch(Exception&)
    {
        OSL_ASSERT(false);
    }

    return ret;
}

// -----------------------------------------------------------------------
// eof
