#ifndef INCLUDED_SETTINGSTABLE_HXX
#define INCLUDED_SETTINGSTABLE_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#include <com/sun/star/text/XTextDocument.hpp>

namespace writerfilter {
namespace dmapper
{

struct SettingsTable_Impl
{
public:
    bool m_bRecordChanges;

    SettingsTable_Impl( );
    ~SettingsTable_Impl( );
};

class WRITERFILTER_DLLPRIVATE SettingsTable: public Properties, public Table
{
public:
    SettingsTable_Impl* m_pImpl;

    SettingsTable( );
    ~SettingsTable( );

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    void resolveSprmProps( Sprm& rSprm );

    void ApplyProperties( com::sun::star::uno::Reference< com::sun::star::text::XTextDocument > xDoc );
};
typedef boost::shared_ptr< SettingsTable > SettingsTablePtr;


} }

#endif
