/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataFlavorMapping.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:09:22 $
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

#ifndef INCLUDED_DATAFLAVORMAPPING_HXX_
#define INCLUDED_DATAFLAVORMAPPING_HXX_

#include <com/sun/star/datatransfer/DataFlavor.hpp>

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include <memory>
#include <boost/shared_ptr.hpp>


/* An interface to get the clipboard data in either
   system or OOo format.
 */
class DataProvider
{
public:
  virtual ~DataProvider() {};

  /* Get the clipboard data in the system format.
     The caller has to retain/release the returned
     CFDataRef on demand.
   */
  virtual CFDataRef getSystemData() = 0;

  /* Get the clipboard data in OOo format.
   */
  virtual com::sun::star::uno::Any getOOoData() = 0;
};

typedef std::auto_ptr<DataProvider> DataProviderPtr_t;

//################################

class DataFlavorMapper
{
public:
  /* Initialialize a DataFavorMapper instance. Throws a RuntimeException in case the XMimeContentTypeFactory service
     cannot be created.
   */
  DataFlavorMapper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > rServiceManager);


  /* Map a system data flavor to an OpenOffice data flavor.
     Return an empty string if there is not suiteable
     mapping from a system data flavor to a OpenOffice data
     flavor.
  */
  com::sun::star::datatransfer::DataFlavor systemToOpenOfficeFlavor(CFStringRef systemDataFlavor) const;


  /* Map an OpenOffice data flavor to a system data flavor.
     If there is no suiteable mapping available a dynamic
     data flavor will be generated and returned. This
     function always returns an usable flavor.
  */
  CFStringRef openOfficeToSystemFlavor(const com::sun::star::datatransfer::DataFlavor& oooDataFlavor) const;


  /* Get a data provider which is able to provide the data 'rTransferable' offers in a format that can
     be put on to the system clipboard.
   */
  DataProviderPtr_t getDataProvider(CFStringRef systemFlavor,
                                    const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable > rTransferable) const;



  /* Get a data provider which is able to provide 'systemData' in the OOo expected format.
   */
  DataProviderPtr_t getDataProvider(const CFStringRef systemFlavor, CFDataRef systemData) const;


private:

  /*  Dynamically registers a data flavor for which no standard mapping is defined.
   */
  CFStringRef registerDynamicSystemFlavor(const com::sun::star::datatransfer::DataFlavor& flavor) const;


  /* Translates back a dynamically registered data flavor.
   */
  com::sun::star::datatransfer::DataFlavor resolveDynamicSystemFlavor(CFStringRef flavor) const;


  /* Determines whether a data flavor is a dynamically registered flavor or not.
   */
  inline bool isDynamicSystemFlavor(CFStringRef flavor) const;

  /* Determines if the provided Mime content type is valid.
   */
  bool isValidMimeContentType(const rtl::OUString& contentType) const;

private:
  const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mrServiceManager;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
};

typedef boost::shared_ptr<DataFlavorMapper> DataFlavorMapperPtr_t;

#endif
