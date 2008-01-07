/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: content.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:42:30 $
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
#ifndef _UCBHELPER_CONTENT_HXX
#define _UCBHELPER_CONTENT_HXX

#include "rtl/ref.hxx"

#ifndef _COM_SUN_STAR_UCB_CONTENTCREATIONEXCEPTION_HPP_
#include <com/sun/star/ucb/ContentCreationException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySetInfo;
} } } }

namespace com { namespace sun { namespace star { namespace io {
    class XActiveDataSink;
    class XOutputStream;
    class XInputStream;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XResultSet;
    class XRow;
} } } }

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandEnvironment;
    class XCommandInfo;
    class XContent;
    class XContentIdentifier;
    class XDynamicResultSet;
    class XAnyCompareFactory;
    struct NumberedSortingInfo;
} } } }

namespace ucbhelper
{

//=========================================================================

/**
  * These are the possible values for the parameter eMode of method
  * ucb::Content::createCursor.
  */
enum ResultSetInclude
{
    INCLUDE_FOLDERS_ONLY,
    INCLUDE_DOCUMENTS_ONLY,
    INCLUDE_FOLDERS_AND_DOCUMENTS
};

/**
  * These are the possible values for the parameter eOperation of method
  * ucb::Content::insertNewContent.
  */
enum InsertOperation
{
    InsertOperation_COPY, // copy source data
    InsertOperation_MOVE, // move source data
    InsertOperation_LINK  // create a link to source
};

//=========================================================================

class Content_Impl;

/**
  * This class simplifies access to UCB contents by providing a more
  * convenient API for frequently used functionality then the "raw"
  * UCB-API does.
  */
class UCBHELPER_DLLPUBLIC Content
{
    rtl::Reference< Content_Impl > m_xImpl;

protected:
    ::com::sun::star::uno::Any createCursorAny( const ::com::sun::star::uno::Sequence<
                                                rtl::OUString >& rPropertyNames,
                                                  ResultSetInclude eMode )
            throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Any createCursorAny( const ::com::sun::star::uno::Sequence<
                                                sal_Int32 >& rPropertyHandles,
                                                  ResultSetInclude eMode )
            throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

public:
    /**
      * Constructor.
      */
    Content();

    /**
      * Constructor.
      *
      * @param rURL is the URL of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      */
    Content( const rtl::OUString& rURL,
             const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv )
        throw ( ::com::sun::star::ucb::ContentCreationException,
                ::com::sun::star::uno::RuntimeException );
    /**
      * Constructor.
      *
      * @param rId is the content identifier of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      */
    Content( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::ucb::XContentIdentifier >& rId,
             const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv )
        throw ( ::com::sun::star::ucb::ContentCreationException,
                ::com::sun::star::uno::RuntimeException );
    /**
      * Constructor.
      *
      * @param rContent is the content object of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      */
    Content( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::ucb::XContent >& rContent,
             const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv )
        throw ( ::com::sun::star::ucb::ContentCreationException,
                ::com::sun::star::uno::RuntimeException );
    /**
      * Copy Constructor.
      *
      * @param rContent is the content this content shall be a copy of.
      */
    Content( const Content& rOther );

    /**
      * Destructor.
      */
    ~Content();

    /**
      * Assignment operator.
      *
      * @param rContent is the content this content shall be a copy of.
      */
    Content& operator=( const Content& rOther );

    /**
      * Constructor. This method should be used, if the exception thrown
      * by the direct ctors of this class are to 'expensive' for your
      * application
      *
      * @param rURL is the URL of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      * @param rContent will be filled by this method with the content created.
      * @return true, if the operation was successful - false, otherwise.
      */
    static sal_Bool
    create( const rtl::OUString& rURL,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv,
            Content& rContent );

    /**
      * Constructor. This method should be used, if the exception thrown
      * by the direct ctors of this class are to 'expensive' for your
      * application
      *
      * @param rId is the content identifier of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      * @param rContent will be filled by this method with the content created.
      * @return true, if the operation was successful - false, otherwise.
      */
    static sal_Bool
    create( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::ucb::XContentIdentifier >& rId,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv,
            Content& rContent );

    /**
      * Constructor. This method should be used, if the exception thrown
      * by the direct ctors of this class are to 'expensive' for your
      * application
      *
      * @param xContent is the content object of the content to create.
      * @param rEnv is the environment to use for commands executed by the
      *        content. The command environment is used by the content
      *        implementation to interact with the client and to propagate
      *        errors.
      * @param rContent will be filled by this method with the content created.
      * @return true, if the operation was successful - false, otherwise.
      */
    static sal_Bool
    create( const ::com::sun::star::uno::Reference<
                     ::com::sun::star::ucb::XContent >& xContent,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& rEnv,
            Content& rContent );

    //////////////////////////////////////////////////////////////////////
    // Direct access to UCB content.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method provides access to the "native" UCB content interface(s).
      * This is usefull in case the convenience methods provided by this
      * class are insufficient for your needs. You may obtain all interfaces
      * supported by the underlying UCB content by calling this method and
      * after that doing a queryInterface call.
      *
      * @return the XContent interface of the underlying UCB content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >
    get() const;

    //////////////////////////////////////////////////////////////////////
    // Object identity.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns the URL of the content.
      *
      * @return the URL of the content.
      */
    const ::rtl::OUString& getURL() const;

    //////////////////////////////////////////////////////////////////////
    // Command environment.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns the environment to use when executing commands.
      *
      * @return the command environment.
      */
    const ::com::sun::star::uno::Reference<
            ::com::sun::star::ucb::XCommandEnvironment >&
    getCommandEnvironment() const;

    /**
      * This method sets a new command environment.
      *
      * @param xNewEnv is the new command environment.
      */
    void setCommandEnvironment(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XCommandEnvironment >& xNewEnv );

    //////////////////////////////////////////////////////////////////////
    // Access to supported commands/properties.
    //////////////////////////////////////////////////////////////////////

    /**
      * This methods provides access to meta data of the commands supported
      * by this content.
      *
      * @return an XCommandInfo interface implementation, which can be used
      *         to obtain meta data of the commands supported by this content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandInfo >
    getCommands()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods provides access to meta data of the properties supported
      * by this content.
      *
      * @return an XPropertSetInfo interface implementation, which can be used
      *         to obtain meta data of the properties supported by this content.
      */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySetInfo >
    getProperties()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    //////////////////////////////////////////////////////////////////////
    // Access to property value(s).
    //////////////////////////////////////////////////////////////////////

    /**
      * This method can be used to read a single property value.
      *
      * @param  rPropertyName is the name of the property for that the value
      *         shall be obtained.
      * @return the property value.
      */
    ::com::sun::star::uno::Any
    getPropertyValue( const rtl::OUString& rPropertyName )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to read a single property value.
      *
      * @param  nPropertyHande is the handle of the property for that the
      *         value shall be obtained.
      * @return the property value.
      */
    ::com::sun::star::uno::Any
    getPropertyValue( sal_Int32 nPropertyHandle )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to set a single property value.
      *
      * @param  rPropertyName is the name of the property for that the
      *         value shall be set.
      * @return an any containing:
      *         - No value indicates, that the property value was set
      *           successfully.
      *         - com::sun::star::beans::UnknownPropertyException indicates,
      *           that the property is not known to the content implementation.
      *         - com::sun::star::beans::IllegalTypeException indicates, that
      *           the data type of the property value is not acceptable.
      *         - com::sun::star::lang::IllegalAccessException indicates, that
      *           the property is constant.
      *         - com::sun::star::lang::IllegalArgumentException indicates,
      *           that the property value is not acceptable. For instance,
      *           setting an empty title may be illegal.
      *         - Any other execption derived from
      *           com::sun::star::uno::Exception indicates, that the value was
      *           not set successfully. For example, this can be a
      *           com::sun:star::ucb::InteractiveAugmentedIOException
      *           transporting the error code
      *           com::sun::star::ucb::IOErrorCode::ACCESS_DENIED.
      */
    ::com::sun::star::uno::Any
    setPropertyValue( const rtl::OUString& rPropertyName,
                      const ::com::sun::star::uno::Any& rValue )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to set a single property value.
      *
      * @param  nPropertyHande is the handle of the property for that the
      *         value shall be set.
      * @return an any containing:
      *         - No value indicates, that the property value was set
      *           successfully.
      *         - com::sun::star::beans::UnknownPropertyException indicates,
      *           that the property is not known to the content implementation.
      *         - com::sun::star::beans::IllegalTypeException indicates, that
      *           the data type of the property value is not acceptable.
      *         - com::sun::star::lang::IllegalAccessException indicates, that
      *           the property is constant.
      *         - com::sun::star::lang::IllegalArgumentException indicates,
      *           that the property value is not acceptable. For instance,
      *           setting an empty title may be illegal.
      *         - Any other execption derived from
      *           com::sun::star::uno::Exception indicates, that the value was
      *           not set successfully. For example, this can be a
      *           com::sun:star::ucb::InteractiveAugmentedIOException
      *           transporting the error code
      *           com::sun::star::ucb::IOErrorCode::ACCESS_DENIED.
      */
    ::com::sun::star::uno::Any
    setPropertyValue( const sal_Int32 nPropertyHandle,
                      const ::com::sun::star::uno::Any& rValue )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to read multiple property values.
      *
      * @param  rPropertyNames is a sequence of names of properties for
      *         that the values shall be obtained.
      * @return the property values.
      */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                                rtl::OUString >& rPropertyNames )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to read multiple property values.
      *
      * @param  nPropertyHandles is a sequence of handles of properties for
      *         that the values shall be obtained.
      * @return an XRow interface that can be used to obtain the property
      *         values.
      */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    getPropertyValues( const ::com::sun::star::uno::Sequence<
                                sal_Int32 >& nPropertyHandles )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to read multiple property values.
      *
      * @param  rPropertyNames is a sequence of names of properties for
      *         that the values shall be obtained.
      * @return the property values.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValuesInterface( const ::com::sun::star::uno::Sequence<
                                rtl::OUString >& rPropertyNames )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to read multiple property values.
      *
      * @param  nPropertyHandles is a sequence of handles of properties for
      *         that the values shall be obtained.
      * @return an XRow interface that can be used to obtain the property
      *         values.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
    getPropertyValuesInterface( const ::com::sun::star::uno::Sequence<
                                sal_Int32 >& nPropertyHandles )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to set multiple property values.
      *
      * @param  rPropertyNames is a sequence of names of properties for
      *         that values shall be set.
      * @return a sequence of any's which has exactly the same number
      *         of elements as the number of properties to set. Every
      *         sequence element contains the status for a property. The
      *         first sequence elements corresponds to the first element in
      *         the sequence of property names and so on.
      *
      *         An any containing:
      *         - No value indicates, that the property value was set
      *           successfully.
      *         - com::sun::star::beans::UnknownPropertyException indicates,
      *           that the property is not known to the content implementation.
      *         - com::sun::star::beans::IllegalTypeException indicates, that
      *           the data type of the property value is not acceptable.
      *         - com::sun::star::lang::IllegalAccessException indicates, that
      *           the property is constant.
      *         - com::sun::star::lang::IllegalArgumentException indicates,
      *           that the property value is not acceptable. For instance,
      *           setting an empty title may be illegal.
      *         - Any other execption derived from
      *           com::sun::star::uno::Exception indicates, that the value was
      *           not set successfully. For example, this can be a
      *           com::sun:star::ucb::InteractiveAugmentedIOException
      *           transporting the error code
      *           com::sun::star::ucb::IOErrorCode::ACCESS_DENIED.
      */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                                rtl::OUString >& rPropertyNames,
                       const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Any >& rValues )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to set multiple property values.
      *
      * @param  nPropertyHandles is a sequence of handles of properties for
      *         that values shall be set.
      * @return a sequence of any's which has exactly the same number
      *         of elements as the number of properties to set. Every
      *         sequence element contains the status for a property. The
      *         first sequence elements corresponds to the first element in
      *         the sequence of property names and so on.
      *
      *         An any containing:
      *         - No value indicates, that the property value was set
      *           successfully.
      *         - com::sun::star::beans::UnknownPropertyException indicates,
      *           that the property is not known to the content implementation.
      *         - com::sun::star::beans::IllegalTypeException indicates, that
      *           the data type of the property value is not acceptable.
      *         - com::sun::star::lang::IllegalAccessException indicates, that
      *           the property is constant.
      *         - com::sun::star::lang::IllegalArgumentException indicates,
      *           that the property value is not acceptable. For instance,
      *           setting an empty title may be illegal.
      *         - Any other execption derived from
      *           com::sun::star::uno::Exception indicates, that the value was
      *           not set successfully. For example, this can be a
      *           com::sun:star::ucb::InteractiveAugmentedIOException
      *           transporting the error code
      *           com::sun::star::ucb::IOErrorCode::ACCESS_DENIED.
      */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    setPropertyValues( const ::com::sun::star::uno::Sequence<
                                sal_Int32 >& nPropertyHandles,
                       const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Any >& rValues )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    //////////////////////////////////////////////////////////////////////
    // General command execution.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method can be used to execute any command supported by the
      * content.
      *
      * @param  rCommandName is the name of the command to execute.
      * @param  rCommandArgument is the argument for the command. Type and
      *         values of this parameter must correspond to the command
      *         specification.
      * @return the result of the command according to its specification.
      */
    ::com::sun::star::uno::Any
    executeCommand( const rtl::OUString& rCommandName,
                    const ::com::sun::star::uno::Any& rCommandArgument )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    ::com::sun::star::uno::Any
    /**
      * This method can be used to execute any command supported by the
      * content.
      *
      * @param  rCommandHandle is the handle of the command to execute.
      * @param  rCommandArgument is the argument for the command. Type and
      *         values of this parameter must correspond to the command
      *         specification.
      * @return the result of the command according to its specification.
      */
    executeCommand( sal_Int32 nCommandHandle,
                    const ::com::sun::star::uno::Any& rCommandArgument )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method can be used to abort the command currently executed by
      * a content. Note that a content can only process one command per
      * thread at a time. The implementation of the content is responsible
      * for determining the command to abort when this method is called.
      */
    void
    abortCommand();

    //////////////////////////////////////////////////////////////////////
    // Special commands.
    //////////////////////////////////////////////////////////////////////

    /**
      * This methods gives access to the children of a folder content.
      * Additionally, the result set returned provides efficient access to
      * preselected property values of the children.
      * Internally it executes the command "open" at the content.
      *
      * @param  rPropertyNames is a sequence of names of properties for
      *         that the values should be accessible via the resultset
      *         returned by this method.
      * @param  eMode is a very simple filter for the children contained
      *         in the resultset.
      * @return an implementation of the service
      *         com.cun.star.ucb.ContentResultSet, which can be used to
      *         get access to the children of a content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
    createCursor( const ::com::sun::star::uno::Sequence<
                                rtl::OUString >& rPropertyNames,
                  ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives access to the children of a folder content.
      * Additionally, the result set returned provides efficient access to
      * preselected property values of the children.
      * Internally it executes the command "open" at the content.
      *
      * @param nPropertyHandles is a sequence of handles of properties for
      *        that the values should be accessible via the resultset
      *        returned by this method.
      * @param eMode is a very simple filter for the children contained
      *        in the resultset.
      * @return an implementation of the service
      *         com.cun.star.ucb.ContentResultSet, which can be used to
      *         get access to the children of a content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
    createCursor( const ::com::sun::star::uno::Sequence<
                                sal_Int32 >& rPropertyHandles,
                  ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives access to the children of a folder content.
      * Additionally, the result set returned provides efficient access to
      * preselected property values of the children.
      * Internally it executes the command "open" at the content.
      *
      * @param rPropertyNames is a sequence of names of properties for
      *        that the values should be accessible via the resultset
      *        returned by this method.
      * @param eMode is a very simple filter for the children contained
      *        in the resultset.
      * @return an implementation of the service
      *         com.cun.star.ucb.DynamicResultSet, which can be used to
      *         get access to the children of a content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet >
    createDynamicCursor( const ::com::sun::star::uno::Sequence<
                         rtl::OUString >& rPropertyNames,
                           ResultSetInclude eMode
                             = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives access to the children of a folder content.
      * Additionally, the result set returned provides efficient access to
      * preselected property values of the children.
      * Internally it executes the command "open" at the content.
      *
      * @param nPropertyHandes is a sequence of handles of properties for
      *        that the values should be accessible via the resultset
      *        returned by this method.
      * @param eMode is a very simple filter for the children contained
      *        in the resultset.
      * @return an implementation of the service
      *         com.cun.star.ucb.DynamicResultSet, which can be used to
      *         get access to the children of a content.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet >
    createDynamicCursor( const ::com::sun::star::uno::Sequence<
                         sal_Int32 >& rPropertyHandles,
                           ResultSetInclude eMode
                             = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet >
    createSortedDynamicCursor( const ::com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames,
                               const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                               ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompareFactory > rAnyCompareFactory,
                               ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XDynamicResultSet >
    createSortedDynamicCursor( const ::com::sun::star::uno::Sequence< sal_Int32 >& rPropertyHandles,
                               const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                               ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompareFactory > rAnyCompareFactory,
                               ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
    createSortedCursor( const ::com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompareFactory > rAnyCompareFactory,
                        ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
    createSortedCursor( const ::com::sun::star::uno::Sequence< sal_Int32 >& rPropertyHandles,
                               const ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::NumberedSortingInfo >& rSortInfo,
                               ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompareFactory > rAnyCompareFactory,
                               ResultSetInclude eMode = INCLUDE_FOLDERS_AND_DOCUMENTS )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    /**
      * This methods gives read access to the content stream of a content (i.e
      * the content of a file located at the local file system).
      * Internally it executes the command "open" at the content.
      *
      * @return an implementation of the interface XInputStream, which can
      *         be used to read the content's data.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
    openStream()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives read/write access to the content stream of a content (i.e
      * the content of a file located at the local file system).
      * Internally it executes the command "open" at the content.
      *
      * @return an implementation of the interface XStream, which can
      *         be used to read/write the content's data.
      */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
    openWriteableStream()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives read access to the content stream of a content (i.e
      * the content of a file located at the local file system).
      * Internally it executes the command "open" at the content.
      *
      * @param rSink is the implementation of an XActiveDataSink interface,
      *        which shall be used by the content to deliver the data.
      */
    sal_Bool
    openStream( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XActiveDataSink >& rSink )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives read access to the content stream of a content (i.e
      * the content of a file located at the local file system).
      * Internally it executes the command "open" at the content.
      *
      * @param rStream is the implementation of an XOutputStream interface,
      *        which shall be used by the content to deliver the data.
      */
    sal_Bool
    openStream( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::io::XOutputStream >& rStream )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This methods gives write access to the content stream of a content (i.e
      * the content of a file located at the local file system).
      * Internally it executes the command "insert" at the content.
      *
      * @param rStream is the implementation of an XInputStream interface,
      *        which contains the content data to write.
      * @param bReplaceExisting specifies, whether any existing content data
      *        shall be overwritten.
      */
    void
    writeStream( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::io::XInputStream >& rStream,
                 sal_Bool bReplaceExisting )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method creates, initializes and inserts ( commits ) a new content
      * (i.e. it could be used to create a new file system folder).
      * Internally this method does a XContentCreator::createNewContent(...)-
      * XCommandProcessor::execute( "setPropertyValues", ... )-
      * XCommandProcessor::execute( "insert", ... ) calling sequence.
      *
      * @param rContentType is the type for the new UCB content. Each content
      *        provider implementation may introduce own types for its contnt
      *        objects.
      * @param rPropertyNames is a sequence of names of properties for that
      *        values are to set at the new content before it will be inserted
      *        ( commited ).
      *        The order of the names must correspond to the order of the
      *        property values.
      * @param rPropertyValues is a sequence of property values that are to
      *        set at the new content before it will be inserted ( commited ).
      *        The order of the values must correspond to the order of the
      *        property names.
      * @param rNewContent will be filled by the implementation of this method
      *        with the new content.
      */
    sal_Bool
    insertNewContent( const ::rtl::OUString& rContentType,
                      const ::com::sun::star::uno::Sequence<
                          rtl::OUString >& rPropertyNames,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Any >& rPropertyValues,
                      Content& rNewContent )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method creates, initializes and inserts ( commits ) a new content
      * (i.e. it could be used to create a new file system folder).
      * Internally this method does a XContentCreator::createNewContent(...)-
      * XCommandProcessor::execute( "setPropertyValues", ... )-
      * XCommandProcessor::execute( "insert", ... ) calling sequence.
      *
      * @param rContentType is the type for the new UCB content. Each content
      *        provider implementation may introduce own types for its contnt
      *        objects.
      * @param nPropertyHandes is a sequence of handles of properties for that
      *        values are to set at the new content before it will be inserted
      *        ( commited ).
      *        The order of the handles must correspond to the order of the
      *        property values.
      * @param rPropertyValues is a sequence of property values that are to
      *        set at the new content before it will be inserted ( commited ).
      *        The order of the values must correspond to the order of the
      *        property handles.
      * @param rNewContent will be filled by the implementation of this method
      *        with the new content.
      */
    sal_Bool
    insertNewContent( const ::rtl::OUString& rContentType,
                      const ::com::sun::star::uno::Sequence<
                        sal_Int32 >& nPropertyHandles,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Any >& rPropertyValues,
                      Content& rNewContent )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method creates, initializes and inserts (commits) a new content
      * inside this (the target folder) content. For example, it can be used to
      * create a new file system folder.
      * Internally this method does a XContentCreator::createNewContent(...)-
      * XCommandProcessor::execute( "setPropertyValues", ... )-
      * XCommandProcessor::execute( "insert", ... ) calling sequence.
      *
      * @param rContentType is the type for the new UCB content. Each content
      *        provider implementation may introduce own types for its contnt
      *        objects.
      * @param rPropertyNames is a sequence of names of properties for that
      *        values are to set at the new content before it will be inserted
      *        ( commited ).
      *        The order of the names must correspond to the order of the
      *        property values.
      * @param rPropertyValues is a sequence of property values that are to
      *        set at the new content before it will be inserted ( commited ).
      *        The order of the values must correspond to the order of the
      *        property names.
      * @param rStream is a stream containing the content data for the new
      *        content (i.e. the content of a file to create)
      * @param rNewContent will be filled by the implementation of this method
      *        with the new content.
      */
    sal_Bool
    insertNewContent( const ::rtl::OUString& rContentType,
                      const ::com::sun::star::uno::Sequence<
                          rtl::OUString >& rPropertyNames,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Any >& rPropertyValues,
                      const ::com::sun::star::uno::Reference<
                                ::com::sun::star::io::XInputStream >& rStream,
                      Content& rNewContent )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method creates, initializes and inserts (commits) a new content
      * inside this (the target folder) content. For example, it can be used to
      * create a new file system folder.
      * Internally this method does a XContentCreator::createNewContent(...)-
      * XCommandProcessor::execute( "setPropertyValues", ... )-
      * XCommandProcessor::execute( "insert", ... ) calling sequence.
      *
      * @param rContentType is the type for the new UCB content. Each content
      *        provider implementation may introduce own types for its contnt
      *        objects.
      * @param nPropertyHandes is a sequence of handles of properties for that
      *        values are to set at the new content before it will be inserted
      *        ( commited ).
      *        The order of the handles must correspond to the order of the
      *        property values.
      * @param rPropertyValues is a sequence of property values that are to
      *        set at the new content before it will be inserted ( commited ).
      *        The order of the values must correspond to the order of the
      *        property handles.
      * @param rStream is a stream containing the content data for the new
      *        content (i.e. the content of a file to create)
      * @param rNewContent will be filled by the implementation of this method
      *        with the new content.
      */
    sal_Bool
    insertNewContent( const ::rtl::OUString& rContentType,
                      const ::com::sun::star::uno::Sequence<
                        sal_Int32 >& nPropertyHandles,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Any >& rPropertyValues,
                      const ::com::sun::star::uno::Reference<
                                ::com::sun::star::io::XInputStream >& rStream,
                      Content& rNewContent )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method transfers (copies/moves) a content. It creates a new
      * resource inside this (the target folder) content.
      * The implementation is able to do cross-provider transfers (like copying
      * a file from the local file system to a directory located on an HTTP
      * server).
      * Internally this method executes the command "globalTransfer" at the UCB.
      *
      * @param rSourceContent is the content that contains the data for the
      *        new UCB content.
      * @param eOperation defines what shall be done with the source data
      *        ( COPY, MOVE, LINK ).
      * @param rTitle contains a title for the new content. If this is an empty
      *        string, the new content will have the same title as the source
      *        content.
      * @param rNameClashAction describes how the implementation shall behave
      *        in case a content with a clashing name exists in the target
      *        folder.
      *        NameClash::ERROR will abort the operation, NameClash::OVERWRITE
      *        will overwrite the clashing content and all its data,
      *        NameClash::RENAME will generate and supply a non-clashing title.
      *        @see com/sun/star/ucb/NameClash.idl
      */
    sal_Bool
    transferContent( const Content& rSourceContent,
                     InsertOperation eOperation,
                     const ::rtl::OUString & rTitle,
                     const sal_Int32 nNameClashAction )
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );

    //////////////////////////////////////////////////////////////////////
    // Required properties.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns the value of the content's property "IsFolder".
      *
      * @return true, if the content is a folder ( it can contain other
      *         UCB contents). false, otherwise.
      */
    sal_Bool
    isFolder()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
    /**
      * This method returns the value of the content's property "IsDocument".
      *
      * @return true, if the content is a document ( it has a content stream ).
      *         false, otherwise.
      */
    sal_Bool
    isDocument()
        throw( ::com::sun::star::ucb::CommandAbortedException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception );
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_CONTENT_HXX */
