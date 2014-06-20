package org.apache.openoffice.ooxml.framework.part;

import java.util.HashMap;
import java.util.Map;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.framework.part.parser.ParserFactory;
import org.apache.openoffice.ooxml.parser.ElementContext;
import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;

public class ContentTypes
{
    ContentTypes (final PartManager aPartManager)
    {
        maExtensionToContentTypeMap = new HashMap<>();
        maPartNameToContentTypeMap = new HashMap<>();

        // Technically the [Content_Types].xml stream is not a part and
        // "[Content_Types].xml" is not a part name.  But handling it like one
        // makes the implementation a little bit easier and more readable.
        final Part aContentTypesPart = new Part(
            ContentType.ContentTypes,
            aPartManager,
            new PartName("/[Content_Types].xml"));

        final Parser aParser = ParserFactory.getParser(
            aContentTypesPart.getContentType(),
            aContentTypesPart.getStream(),
            null);
        /*
        DefineContext(
            CT_Something,
            int nValue,
            CallbackObject aObject);

        class CT_Something_Context : public Context
        {
            Context parent

            attribute 1
            ...
            attribute n
            int nValue;
            CallbackObject aObject;
        }

        DefineElementStartAction(
            CT_Something_Context,
            aObject,
            DoSomething);


        case ElementStart of CT_Something:
            maCurrentContext.aObject.DoSomething(maCurrentContext);    // CT_Something_Context

        //
        CallbackObject.cxx

        class CallbackObject
        {
            public: DoSomething(CT_Something_Context aContext)
            {
                aContext.attribute1
            }
        }

            */
        aParser.GetActionManager().AddElementStartAction(
            ".*_CT_Default",
            new IAction(){

                @Override
                public void Run(
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aStartLocation,
                    final Location aEndLocation)
                {
                    ProcessDefault(
                        aContext.GetAttributes().GetRawAttributeValue("A_Extension"),
                        aContext.GetAttributes().GetRawAttributeValue("A_ContentType"));

                }});
        aParser.GetActionManager().AddElementStartAction(
            ".*_CT_Override",
            new IAction(){

                @Override
                public void Run(
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aStartLocation,
                    final Location aEndLocation)
                {
                    ProcessOverride(
                        aContext.GetAttributes().GetRawAttributeValue("A_PartName"),
                        aContext.GetAttributes().GetRawAttributeValue("A_ContentType"));

                }});


        aParser.Parse();
    }




    public ContentType getTypeForPartName (final PartName aName)
    {
        ContentType eType = maPartNameToContentTypeMap.get(aName.GetFullname());
        if (eType == null)
            eType = maExtensionToContentTypeMap.get(aName.GetExtension());
        if (eType == null)
            eType = ContentType.Unknown;
        return eType;
    }




    private void ProcessDefault (
        final String sExtension,
        final String sContentTypeName)
    {
        final ContentType eType = ContentType.CreateForString(sContentTypeName);
        maExtensionToContentTypeMap.put(sExtension, eType);
    }




    private void ProcessOverride (
        final String sPartName,
        final String sContentTypeName)
    {
        final ContentType eType = ContentType.CreateForString(sContentTypeName);
        maPartNameToContentTypeMap.put(sPartName, eType);
    }




    private final Map<String,ContentType> maExtensionToContentTypeMap;
    private final Map<String,ContentType> maPartNameToContentTypeMap;
}
