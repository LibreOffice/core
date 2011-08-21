/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_GRAPHICHELPERS_HXX
#define INCLUDED_GRAPHICHELPERS_HXX

#include "PropertyMap.hxx"

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>

#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{

class WRITERFILTER_DLLPRIVATE PositionHandler: public LoggedProperties
{
public:
    PositionHandler( bool vertical );
    ~PositionHandler( );
    static void setPositionOffset(const ::rtl::OUString & sText, bool vertical);
    static void setAlignH(const ::rtl::OUString & sText);
    static void setAlignV(const ::rtl::OUString & sText);

    sal_Int16 m_nOrient;
    sal_Int16 m_nRelation;
    sal_Int32 m_nPosition;

 private:
    virtual void lcl_attribute( Id aName, Value& rVal );
    virtual void lcl_sprm( Sprm& rSprm );
    static int savedPositionOffsetV, savedPositionOffsetH;
    static int savedAlignV, savedAlignH;
};
typedef boost::shared_ptr<PositionHandler> PositionHandlerPtr;

class WRITERFILTER_DLLPRIVATE WrapHandler: public LoggedProperties
{
public:
    WrapHandler( );
    ~WrapHandler( );

    sal_Int32 m_nType;
    sal_Int32 m_nSide;

    sal_Int32 getWrapMode( );

 private:
    virtual void lcl_attribute( Id aName, Value& rVal );
    virtual void lcl_sprm( Sprm& rSprm );
};
typedef boost::shared_ptr<WrapHandler> WrapHandlerPtr;

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
