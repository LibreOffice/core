Internationalisation (i18npool) framework ensures that the suite is adaptable to the requirements of different
native languages, their local settings and customs, etc without source code modification. (Wow, that is such marketing-speak...)

Specifically for locale data documentation please see i18npool/source/localedata/data/locale.dtd

See also [http://wiki.documentfoundation.org/Category:I18n]

On iOS we put the largest data generated here, the dict_ja and dict_zh
stuff, into separate files and not into code to keep the size of an
app binary down. Temporary test code:

    static bool beenhere = false;
    if (!beenhere) {
        beenhere = true;
        uno::Reference< uno::XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());
        uno::Reference< lang::XMultiComponentFactory > xMultiComponentFactoryClient( xComponentContext->getServiceManager() );
        uno::Reference< uno::XInterface > xInterface =
            xMultiComponentFactoryClient->createInstanceWithContext( "com.sun.star.i18n.BreakIterator_ja", xComponentContext );
    }
