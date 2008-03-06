#ifndef EDITOR_HXX
#define EDITOR_HXX

#include <layout/layout.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class EditorImpl;

class Editor : public layout::Dialog
{
EditorImpl *mpImpl;

public:
    Editor( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xMSF,
            rtl::OUString aFile );
    ~Editor();

    void loadFile( const rtl::OUString &aTestFile );
};

#endif /*EDITOR_HXX*/
