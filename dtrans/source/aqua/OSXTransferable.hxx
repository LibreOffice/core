/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OSXTransferable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:10:32 $
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


#ifndef _TRANSFERABLE_HXX_
#define _TRANSFERABLE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif

#include "DataFlavorMapping.hxx"

#include <premac.h>
    #include <Carbon/Carbon.h>
    #include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include <boost/shared_ptr.hpp>
#include <vector>

struct ClipboardItem
{
  ClipboardItem(PasteboardItemID id,
                CFStringRef systemFlavor,
                const com::sun::star::datatransfer::DataFlavor& oOOFlavor);

  ~ClipboardItem();

  PasteboardItemID mId;
  CFStringRef mSystemDataFlavor;
  com::sun::star::datatransfer::DataFlavor mOOoDataFlavor;
};

typedef boost::shared_ptr<ClipboardItem> ClipboardItemPtr_t;
typedef std::vector<ClipboardItemPtr_t> ClipboardItemContainer_t;


class OSXTransferable : public ::cppu::WeakImplHelper1<com::sun::star::datatransfer::XTransferable>
{
public:
  typedef com::sun::star::uno::Sequence< sal_Int8 > ByteSequence_t;

  explicit OSXTransferable(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > ServiceManager,
                           ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> rXMimeCntFactory,
                           DataFlavorMapperPtr_t pDataFlavorMapper);

  virtual ~OSXTransferable();

  //------------------------------------------------------------------------
  // XTransferable
  //------------------------------------------------------------------------

  virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
    throw( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
    throw( ::com::sun::star::uno::RuntimeException );

  virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
    throw( ::com::sun::star::uno::RuntimeException );

  void addClipboardItemFlavors(PasteboardItemID clipboardItemId);

  ClipboardItemPtr_t findMatchingClipboardItem(const com::sun::star::datatransfer::DataFlavor& aFlavor);

  void initClipboardItemList();

  com::sun::star::uno::Any getClipboardItemData(ClipboardItemPtr_t clipboardItem);

  bool isUnicodeText(ClipboardItemPtr_t clipboardItem);

  bool compareDataFlavors( const com::sun::star::datatransfer::DataFlavor& lhs,
                                    const com::sun::star::datatransfer::DataFlavor& rhs );


  bool cmpAllContentTypeParameter( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType > xLhs,
                                   const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType > xRhs ) const;


private:
  ClipboardItemContainer_t mClipboardItems;
  com::sun::star::uno::Sequence< com::sun::star::datatransfer::DataFlavor > m_FlavorList;
  const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mrSrvMgr;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  DataFlavorMapperPtr_t mpDataFlavorMapper;
  osl::Mutex m_aMutex;
  PasteboardRef mrClipboard;

  // prevent copy and assignment
private:
  OSXTransferable( const OSXTransferable& );
  OSXTransferable& operator=( const OSXTransferable& );
};

#endif
