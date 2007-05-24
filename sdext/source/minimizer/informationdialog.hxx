/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: informationdialog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-24 10:08:36 $
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

#ifndef INFORMATIONDIALOG_HXX
#define INFORMATIONDIALOG_HXX
#include <vector>
#ifndef UNODIALOG_HXX
#include "unodialog.hxx"
#endif
#ifndef _CONFIGURATION_ACCESS_HXX_
#include "configurationaccess.hxx"
#endif
#ifndef _PPPOPTIMIZER_TOKEN_HXX
#include "pppoptimizertoken.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XITEMEVENTBROADCASTER_HPP_
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_PUSHBUTTONTYPE_HPP_
#include <com/sun/star/awt/PushButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

// ---------------------
// - InformationDialog -
// ---------------------
class InformationDialog : public UnoDialog, public ConfigurationAccess
{
public :

    InformationDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF,
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rxFrame, const rtl::OUString& rSaveAsURL,
                sal_Bool& bOpenNewDocument, const sal_Int64& nSourceSize, const sal_Int64& nDestSize, const sal_Int64& nApproxDest );
    ~InformationDialog();

    sal_Bool                execute();

private :

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >mxMSF;
    com::sun::star::uno::Reference< com::sun::star::frame::XFrame >         mxFrame;
    com::sun::star::uno::Reference< com::sun::star::io::XStream >           mxTempFile;

    com::sun::star::uno::Reference< com::sun::star::awt::XActionListener >  mxActionListener;

    rtl::OUString ImpGetStandardImage( const rtl::OUString& rPrivateURL );
    void InitDialog();

    sal_Int64 mnSourceSize;
    sal_Int64 mnDestSize;
    sal_Int64 mnApproxSize;
    sal_Bool& mrbOpenNewDocument;
    const rtl::OUString& maSaveAsURL;

public :

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame>& GetFrame() { return mxFrame; };
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& GetComponentContext() { return mxMSF; };
};

class OKActionListener : public ::cppu::WeakImplHelper1< com::sun::star::awt::XActionListener >
{
public:
    OKActionListener( InformationDialog& rInformationDialog ) : mrInformationDialog( rInformationDialog ){};

    virtual void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& Event ) throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( com::sun::star::uno::RuntimeException);
private:

    InformationDialog& mrInformationDialog;
};

#endif
