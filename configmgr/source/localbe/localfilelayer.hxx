/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localfilelayer.hxx,v $
 * $Revision: 1.13 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
#define CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

#include "propertysethelper.hxx"
#include <com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#include <com/sun/star/configuration/backend/XCompositeLayer.hpp>
#include <com/sun/star/util/XTimeStamped.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#ifndef VECTOR_INCLUDED_
#define VECTOR_INCLUDED_
#include <vector>
#endif // VECTOR_INCLUDED_

namespace configmgr { namespace localbe {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace util = css::util ;
namespace backend = css::configuration::backend ;

/**
  Basic Implementation of the readonly XLayer interfaces for a local file access.
  The read data is accessible through a canned implementation of an XML parser.
  The layer is defined by the URL of the file containing its contents.
  */
class BasicLocalFileLayer
{
protected :
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An optional resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aComponentFile path describing the component file
      */
    BasicLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aComponentFile) ;
    /** Destructor */
    ~BasicLocalFileLayer() ;

protected :
    /**
      Describes the contents of a particular file to a handler.

      @param xHandler   handler to describe the data to
      @param aFileUrl   URL of the file
      @throws com::sun::star::configuration::backend::MalformedDataException
              if the file contains invalid data.
      @throws com::sun::star::lang::NullPointerException
              if pContext is NULL.
      @throws com::sun::star::lang::WrappedTargetException
              if an error occurs while accessing the data.
      */
    void readData(backend::XLayer * pContext,
                  const uno::Reference<backend::XLayerHandler>& xHandler,
                  const rtl::OUString& aFileUrl)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    /**
      Returns an object that can be used to write a layer.
      */
    uno::Reference<backend::XLayerHandler> createLayerWriter();

public:
    /**
      Returns a timestamp associated to a file defined by its URL.

      @param aFileUrl   URL of the file
      @return   timestamp
      */
    static rtl::OUString getTimestamp(const rtl::OUString& aFileUrl) ;

    rtl::OUString const & getFileUrl() const { return mFileUrl; };

private :
    /** Service factory */
    uno::Reference<lang::XMultiServiceFactory> const mFactory ;
    /** URL of the file being accessed */
    rtl::OUString const mFileUrl ;
    /** XLayer implementation used for readData */
    uno::Reference<backend::XLayer> mLayerReader ;

} ;

// provides properties for file layers
class LayerPropertyHelper : public apihelper::PropertySetHelper
{
protected:
    LayerPropertyHelper(){};
    virtual ~LayerPropertyHelper(){};

protected:
    virtual rtl::OUString const & getLayerUrl() const = 0;

protected:
    // cppu::OPropertySetHelper
    virtual cppu::IPropertyArrayHelper * SAL_CALL newInfoHelper();

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue )
            throw (uno::Exception);

    using PropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const;

};

/**
  Implementation of the readonly XLayer interfaces for a local file access.
  The read data is accessible through a canned implementation of
  an XML parser.
  The layer is defined by the URL of the file containing its
  contents.
  */
class SimpleLocalFileLayer  : public BasicLocalFileLayer
                             , public cppu::ImplInheritanceHelper2< LayerPropertyHelper,
                                                                    backend::XLayer,
                                                                    util::XTimeStamped>
{
public :
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An optional resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aComponentFile URL describing the component file
      */
    SimpleLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aComponentFile) ;
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An optional resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aBaseDir   base directory
      @param aComponent subpath describing the component file
      */
    SimpleLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aBaseDir,
            const rtl::OUString& aComponent) ;

    /** Destructor */
    ~SimpleLocalFileLayer() ;

    // XLayer
    using BasicLocalFileLayer::readData;
    virtual void SAL_CALL readData(
            const uno::Reference<backend::XLayerHandler>& xHandler)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp()
        throw (uno::RuntimeException);

protected:
    virtual rtl::OUString const & getLayerUrl() const
    { return getFileUrl(); }
} ;

/**
  Implementation of the XUpdatableLayer
  interface for a local file access.
  The read data is accessible through a canned implementation of
  an XML parser, and the write data is defined through a canned
  implementation of an XML writer.
  The layer is defined by the URL of the file containing its
  contents, and that file will be either read or updated by
  the access to the handlers.
  The timestamp is refreshed on each read operation only.
  */
class FlatLocalFileLayer : public BasicLocalFileLayer
                         , public cppu::ImplInheritanceHelper2< LayerPropertyHelper,
                                                                backend::XUpdatableLayer,
                                                                util::XTimeStamped>
{
public :
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.

      @param xFactory   service factory used to access canned services
      @param aBaseDir   base directory
      @param aComponent subpath describing the component file
      */
    FlatLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aBaseDir,
            const rtl::OUString& aComponent) ;
    /** Destructor */
    ~FlatLocalFileLayer(void) ;

    // XLayer
    using BasicLocalFileLayer::readData;
    virtual void SAL_CALL readData(
            const uno::Reference<backend::XLayerHandler>& xHandler)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XUpdatableLayer
    virtual void SAL_CALL replaceWith(
            const uno::Reference<backend::XLayer>& aNewLayer)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp()
        throw (uno::RuntimeException);

protected:
    virtual rtl::OUString const & getLayerUrl() const
    { return getFileUrl(); }

private :
    /** XLayerHandler implementation for getWriteHandler */
    uno::Reference<backend::XLayerHandler> mLayerWriter ;

} ;

/**
  Implementation of the XCompositeLayer
  interface for a local file access.
  The read data is accessible through a canned implementation of
  an XML parser.
  The layer is defined by the URL of the file containing its
  contents, and that file will be either read or updated by
  the access to the handlers.
  The timestamp is refreshed on each read operation only.
  */
class BasicCompositeLocalFileLayer : public BasicLocalFileLayer
{
protected:
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aComponent path describing the component file
      */
    BasicCompositeLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aComponent) ;

    // XCompositeLayer helpers
    uno::Sequence<rtl::OUString> SAL_CALL listSubLayerIds()
        throw (lang::WrappedTargetException, uno::RuntimeException)
    { return mSubLayers ; }

    void SAL_CALL readSubLayerData(
            backend::XCompositeLayer * context,
            const uno::Reference<backend::XLayerHandler>& xHandler,
            const rtl::OUString& aSubLayerId)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, lang::IllegalArgumentException,
               uno::RuntimeException);

    /**
      Fills the list of available sublayers.

      @param aResDir    resource directory containing potential sublayers
      @param aComponent component subpath
      */
    void fillSubLayerLists(const std::vector<rtl::OUString>& aSublayerDirectories,
                           const rtl::OUString& aComponent) ;
private :
    /** List of available sublayers... */
    uno::Sequence<rtl::OUString> mSubLayers ;
    /** .. and the corresponding file URLs. */
    std::vector<rtl::OUString> mSubLayerFiles ;

};

/**
  Implementation of the XCompositeLayer
  interface for a local file access.
  The read data is accessible through a canned implementation of
  an XML parser.
  The layer is defined by the URL of the file containing its
  contents, and that file will be either read or updated by
  the access to the handlers.
  The timestamp is refreshed on each read operation only.
  */
class CompositeLocalFileLayer : public BasicCompositeLocalFileLayer
                     , public cppu::WeakImplHelper1< backend::XCompositeLayer>
{
public :
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aBaseDir   base directory
      @param aComponent subpath describing the component file
      @param aResDir    resource directory, if empty it is
                        assumed the layer does not have sublayers.
      */
    CompositeLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aComponent,
            const std::vector<rtl::OUString>& aSublayerDirectories) ;
    /** Destructor */
    ~CompositeLocalFileLayer(void) ;
    // XLayer
    using BasicCompositeLocalFileLayer::readData;
    virtual void SAL_CALL readData(
            const uno::Reference<backend::XLayerHandler>& xHandler)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XCompositeLayer
    virtual uno::Sequence<rtl::OUString> SAL_CALL listSubLayerIds()
        throw (lang::WrappedTargetException, uno::RuntimeException)
    { return BasicCompositeLocalFileLayer::listSubLayerIds() ; }

    using BasicCompositeLocalFileLayer::readSubLayerData;
    virtual void SAL_CALL readSubLayerData(
            const uno::Reference<backend::XLayerHandler>& xHandler,
            const rtl::OUString& aSubLayerId)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, lang::IllegalArgumentException,
               uno::RuntimeException);

private :
    // not implemented: warn of attempts to use this here
    void getFileUrl() const;
} ;
/**
  Implementation of the XUpdatableLayer and XCompositeLayer
  interfaces for a local file access.
  The read data is accessible through a canned implementation of
  an XML parser, and the write data is defined through a canned
  implementation of an XML writer.
  The layer is defined by the URL of the file containing its
  contents, and that file will be either read or updated by
  the access to the handlers.
  The timestamp is refreshed on each read operation only.
  */
class FullCompositeLocalFileLayer : public BasicCompositeLocalFileLayer
                                    , public cppu::ImplInheritanceHelper3<
                                                    LayerPropertyHelper,
                                                    backend::XUpdatableLayer,
                                                    backend::XCompositeLayer,
                                                    util::XTimeStamped>
{
public :
    /**
      Constructor providing the base directory and the
      file subpath describing the file to access.
      An resource directory provides the location
      of sublayers of the component.

      @param xFactory   service factory used to access canned services
      @param aBaseDir   base directory
      @param aComponent subpath describing the component file
      @param aResDir    resource directory, if empty it is
                        assumed the layer does not have sublayers.
      */
    FullCompositeLocalFileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aBaseDir,
            const rtl::OUString& aComponent,
            const std::vector<rtl::OUString>& aSublayerDirectories) ;
    /** Destructor */
    ~FullCompositeLocalFileLayer(void) ;
    // XLayer
    using BasicCompositeLocalFileLayer::readData;
    virtual void SAL_CALL readData(
            const uno::Reference<backend::XLayerHandler>& xHandler)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XUpdatableLayer
    virtual void SAL_CALL replaceWith(
            const uno::Reference<backend::XLayer>& aNewLayer)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, uno::RuntimeException);

    // XCompositeLayer
    virtual uno::Sequence<rtl::OUString> SAL_CALL listSubLayerIds()
        throw (lang::WrappedTargetException, uno::RuntimeException)
    { return BasicCompositeLocalFileLayer::listSubLayerIds() ; }

    using BasicCompositeLocalFileLayer::readSubLayerData;
    virtual void SAL_CALL readSubLayerData(
            const uno::Reference<backend::XLayerHandler>& xHandler,
            const rtl::OUString& aSubLayerId)
        throw (backend::MalformedDataException, lang::NullPointerException,
               lang::WrappedTargetException, lang::IllegalArgumentException,
               uno::RuntimeException);

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp()
        throw (uno::RuntimeException);

protected:
    virtual rtl::OUString const & getLayerUrl() const
    { return getFileUrl(); }

private :
    /** XLayerHandler implementation for getWriteHandler */
    uno::Reference<backend::XLayerHandler> mLayerWriter ;
} ;
/**
  Factory function to create the appropriate Flat- or Composite-
  LocalFileLayer for a set of parameters.

  Arguments provide the base directory and the
  file subpath describing the file to access.
  An optional resource directory provides the location
  of sublayers of the component.

  @param xFactory   service factory used to access canned services
  @param aBaseDir   base directory
  @param aComponent subpath describing the component file
  @param aResDir    resource directory, if empty it is
                    assumed the layer does not have sublayers.
  */
uno::Reference<backend::XLayer> createReadonlyLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir) ;

/**
  Factory function to create the appropriate Flat- or Composite-
  LocalFileLayer for a set of parameters.

  Arguments provide the base directory and the
  file subpath describing the file to access.
  An optional resource directory provides the location
  of sublayers of the component.

  @param xFactory   service factory used to access canned services
  @param aBaseDir   base directory
  @param aComponent subpath describing the component file
  @param aResDir    resource directory, if empty it is
                    assumed the layer does not have sublayers.
  */
uno::Reference<backend::XUpdatableLayer> createUpdatableLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir) ;

} } // configmgr.localbe

#endif // CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
