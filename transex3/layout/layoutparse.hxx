#ifndef LAYOUTPARSE_HXX
#define LAYOUTPARSE_HXX

#include "xmlparse.hxx"

class LayoutXMLFile : public XMLFile
{
    bool mMergeMode;

public:
    LayoutXMLFile( bool mergeMode );

    void SearchL10NElements( XMLParentNode *pCur, int pos = 0 );
    BOOL Write( ByteString &aFilename );
    void HandleElement( XMLElement* element );
    void InsertL10NElement( ByteString const& id, XMLElement* element );
};

std::vector<XMLAttribute*> interestingAttributes( XMLAttributeList* lst );

#endif /* LAYOUTPARSE_HXX */
