 /*************************************************************************
 *
 *  $RCSfile: svgimport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:33:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svgfilter.hxx"

// -------------
// - SVGFilter -
// -------------

sal_Bool SVGFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Reference< XMultiServiceFactory >   xServiceFactory( ::comphelper::getProcessServiceFactory() ) ;
    OUString                            aTmpFileName;
    String                              aFileName;
    sal_Int32                           nLength = rDescriptor.getLength();
    const PropertyValue*                pValue = rDescriptor.getConstArray();
    sal_Bool                            bRet = sal_False;

    for( sal_Int32 i = 0 ; ( i < nLength ) && !aTmpFileName.getLength(); i++)
        if( pValue[ i ].Name.equalsAscii( "FileName" ) )
            pValue[ i ].Value >>= aTmpFileName;

    if( aTmpFileName.getLength() && xServiceFactory.is() )
    {
        JavaVM*                 pJVM = NULL;
        Reference< XJavaVM >    xJavaVM( xServiceFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine") ) ), UNO_QUERY );
        Sequence< sal_Int8 >    aProcessID( 16 );
        String                  aLocalFile;

        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( aTmpFileName, aLocalFile ) && aLocalFile.Len() )
        {
            rtl_getGlobalProcessId( (sal_uInt8 *) aProcessID.getArray() );
              Any aVMPtr( xJavaVM->getJavaVM( aProcessID ) );

            if( sizeof( pJVM) == sizeof( sal_Int32 ) )
            {
                sal_Int32 nP = 0;

                aVMPtr >>= nP;
                pJVM = reinterpret_cast< JavaVM* >( nP );
            }
            else if( sizeof( pJVM ) == sizeof( sal_Int64 ) )
            {
                sal_Int64 nP = 0;

                aVMPtr >>= nP;
                pJVM = reinterpret_cast< JavaVM* >( nP );
            }

            if( pJVM )
            {
                jobjectArray    aArgs;
                jclass          aClass;
                jmethodID       aMId;
                jstring         aJStr;

                Reference< XJavaThreadRegister_11 > xJavaThreadRegister_11( xJavaVM, UNO_QUERY );
                  sj2::TKTThreadAttach                aJEnv( pJVM, xJavaThreadRegister_11.get() );

                aClass = aJEnv.pEnv->FindClass( "SOTranscoder" );

                if( aClass && ( aMId = aJEnv.pEnv->GetStaticMethodID( aClass, "main", "([Ljava/lang/String;)V" ) ) )
                {
                    ::utl::TempFile aTempFile;
                    String          aOutputURL( aTempFile.GetURL() );
                    String          aOutputFile;

                    aTempFile.EnableKillingFile();

                    if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( aOutputURL, aOutputFile ) && aOutputFile.Len() )
                    {
                        aJStr = aJEnv.pEnv->NewString( aLocalFile.GetBuffer(),
                                                       aLocalFile.Len() );
                        aArgs = aJEnv.pEnv->NewObjectArray( 2, aJEnv.pEnv->FindClass( "java/lang/String" ), aJStr );
                        aJStr = aJEnv.pEnv->NewString( aOutputFile.GetBuffer(),
                                                       aOutputFile.Len() );
                        aJEnv.pEnv->SetObjectArrayElement( aArgs, 1, aJStr );
                        aJEnv.pEnv->CallStaticVoidMethod( aClass, aMId, aArgs );

                        Graphic     aGraphic;
                        SvStream*   pIStm = ::utl::UcbStreamHelper::CreateStream( aOutputURL, STREAM_READ );

                        if( pIStm )
                        {
                            GraphicConverter::Import( *pIStm, aGraphic );
                            delete pIStm;
                        }

                        Reference< XDrawPagesSupplier > xDrawPagesSupplier( mxDstDoc, UNO_QUERY );

                        if( xDrawPagesSupplier.is() && ( aGraphic.GetType() != GRAPHIC_NONE ) )
                        {
                            Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages() );

                            if( xDrawPages.is() && xDrawPages->getCount() )
                            {
                                Reference< XDrawPage >  xDrawPage;

                                if( xDrawPages->getByIndex( 0 ) >>= xDrawPage )
                                {
                                    Reference< XShapes >        xShapes( xDrawPage, UNO_QUERY );
                                    Reference< XPropertySet>    xPagePropSet( xDrawPage, UNO_QUERY );
                                    Reference< XShape >         xShape( Reference< XMultiServiceFactory >( mxDstDoc, UNO_QUERY )->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GraphicObjectShape" ) ) ), UNO_QUERY );

                                    if( xPagePropSet.is() && xShapes.is() && xShape.is() )
                                    {
                                        Reference< XPropertySet >   xPropSet( xShape, UNO_QUERY );
                                        sal_Int32                   nPageWidth = 0, nPageHeight = 0;

                                        xPagePropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nPageWidth;
                                        xPagePropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nPageHeight;

                                        if( xPropSet.is() && nPageWidth && nPageHeight )
                                        {
                                            xShapes->add( xShape );

                                            ::com::sun::star::awt::Point    aPos;
                                            ::com::sun::star::awt::Size     aSize;
                                            GraphicObject                   aGraphObj( aGraphic );
                                            String                          aGraphURL( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
                                            Any                             aValue;
                                            Size                            aGraphicSize;
                                            const MapMode                   aTargetMapMode( MAP_100TH_MM );

                                            if( aGraphObj.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                                                aGraphicSize = Application::GetDefaultDevice()->PixelToLogic( aGraphObj.GetPrefSize(), aTargetMapMode );
                                            else
                                                aGraphicSize = OutputDevice::LogicToLogic( aGraphObj.GetPrefSize(), aGraphObj.GetPrefMapMode(), aTargetMapMode );

                                            aGraphURL += String( aGraphObj.GetUniqueID(), RTL_TEXTENCODING_ASCII_US );
                                            aValue <<= OUString( aGraphURL );
                                            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), aValue );

                                            aPos.X = ( nPageWidth - aGraphicSize.Width() ) >> 1;
                                            aPos.Y = ( nPageHeight - aGraphicSize.Height() ) >> 1;

                                            aSize.Width = aGraphicSize.Width();
                                            aSize.Height = aGraphicSize.Height();

                                            xShape->setPosition( aPos );
                                            xShape->setSize( aSize );

                                            bRet = sal_True;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return bRet;
}
