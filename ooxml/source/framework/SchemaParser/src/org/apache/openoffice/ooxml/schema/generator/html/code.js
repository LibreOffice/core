function LoadData ()
{
    gTypeNames = [];
    ComplexTypeNames = [];
    SimpleTypeNames = [];
    for (var sName in Data)
    {
        gTypeNames.push(sName);
        switch(Data[sName].type)
        {
            case "complex-type":
                ComplexTypeNames.push(sName);
                break;
                
            case "simple-type":
                SimpleTypeNames.push(sName);
                break;
        }
    }
    document.getElementById("message").innerHTML = "there are " + ComplexTypeNames.length + " complex types and "
        + SimpleTypeNames.length +" simple types";
}




function InitializeSearch ()
{
	CurrentTypeName = "";
	TypeHistory = Array();
    LoadData();
    ShowType("A:ST_Overlap");
}




function CheckInput (aField, aEvent)
{
    switch(aEvent.keyCode)
    {
        case 38:
            --selection_index;
            if (selection_index < 0)
                selection_index = matches.length-1;
            break;
        case 40:
            ++selection_index;
            if (selection_index >= matches.length)
                selection_index = 0;
            break;
        default:
            matches = GetMatches(aField.value);
            selection_index = 0;
   }

    ShowMatches(matches, selection_index);
}




function GetMatches (sPattern)
{
    var aLcPatterns = sPattern.toLowerCase().split(/\s+/);

    var nTypeCount = gTypeNames.length;

    var aMatches = [];
    for (index=0; index<nTypeCount; ++index)
    {
        var sTypeName = gTypeNames[index]; 
        var aParts = new Array(sTypeName);
        var sLcTypeName = sTypeName.toLowerCase();
        var bIsMatch = true;
        var nSearchStart = 0;
        for (nPatternIndex=0; nPatternIndex<aLcPatterns.length && bIsMatch; ++nPatternIndex)
        {
            var sPattern = aLcPatterns[nPatternIndex];
            var nMatchStart = sLcTypeName.indexOf(sPattern, nSearchStart);
            if (nMatchStart >= 0)
            {
                var nMatchEnd = nMatchStart + sPattern.length;
                aParts.push(sTypeName.substring(nSearchStart, nMatchStart));
                aParts.push(sTypeName.substring(nMatchStart, nMatchEnd));
                nSearchStart = nMatchEnd;
            }
            else
            {
                // Only some patterns are matched.
                bIsMatch = false;
            }
        }
        if (bIsMatch)
        {
            if (nMatchEnd < sTypeName.length-1)
                aParts.push(sTypeName.substring(nMatchEnd));
            aMatches.push(aParts);
        }
    }
    return aMatches;
}




/** Show the matching types.
 *  As there can be a great many matching types, some sort of abbreviation is necessary.
 *  Format all matches into lines of n entries.
 *  Show the line containing the selected match and the ones before and after.
 *  Show also the number of ommited matches.
 */
function ShowMatches (aMatches, nSelectionIndex)
{
    var sText = "";
    
    var nHalfRange = 10;
    var nMatchesPerLine = 5;
    var nLineCount = Math.floor((aMatches.length+nMatchesPerLine-1) / nMatchesPerLine);
    var nLineOfSelection = Math.floor(nSelectionIndex / nMatchesPerLine);
    var nFirstDisplayedLine = nLineOfSelection>0 ? nLineOfSelection-1 : 0;
    var nLastDisplayedLine = nLineOfSelection<nLineCount-1 ? nLineOfSelection+1 : nLineCount-1;
    
    for (nLineIndex=nFirstDisplayedLine; nLineIndex<=nLastDisplayedLine; ++nLineIndex)
    {
        var nLineStartIndex = nLineIndex * nMatchesPerLine;
        var nLineEndIndex = nLineStartIndex + nMatchesPerLine - 1;
        if (nLineEndIndex >= aMatches.length)
            nLineEndIndex = aMatches.length-1;
        
        sText += "<tr>"
        for (nIndex=nLineStartIndex; nIndex<=nLineEndIndex; ++nIndex)
        {
            var aMatch = aMatches[nIndex];
            var sTypeName = aMatch[0];
            var sMatch = "";
            for (nPartIndex=1; nPartIndex<aMatch.length; ++nPartIndex)
            {
                if ((nPartIndex%2)==0)
                    sMatch += "<span class=\"match-highlight\">"+aMatch[nPartIndex]+"</span>";
                else
                    sMatch += aMatch[nPartIndex];
            }
            sText += "<td>"
            if (nIndex == nSelectionIndex)
            {
                sText += " <span class=\"typelink current-match\">" + sMatch + "</span>";
            }
            else
            {
                sText += " " + GetTypeLink(sMatch, sTypeName, -1);
            }
            sText += "</td>"
        }
        
        sText += "</tr>";
    }
    if (nFirstDisplayedLine > 0)
        sText = "<tr><td>["+(nFirstDisplayedLine*nMatchesPerLine)+" matches] ...</td><tr>" + sText;
    if (nLastDisplayedLine+1 < nLineCount)
        sText += "<tr><td>... ["+((nLineCount-nLastDisplayedLine-1)*nMatchesPerLine)+" matches]</td></tr>";

    document.getElementById('matches').innerHTML = "<table>"+sText+"</table>";
    if (aMatches.length == 0)
    {
        document.getElementById('match-count').innerHTML = "no match:";
    }
    else
    {
        if (aMatches.length == 1)
            document.getElementById('match-count').innerHTML = "single match:";
        else
            document.getElementById('match-count').innerHTML = aMatches.length+" matches:";

        ShowType(aMatches[nSelectionIndex][0]);
    }
}




function GetTopLevelNodeForTypeName(sTypeName)
{
    if (sTypeName in Data)
    	return Data[sTypeName];
    else
    	alert(sTypeName +" is not a known complex type, simple type, or group");
}




/** Show the specified type.
 *  When nHistoryIndex is not -1 then the history is shortened to that (before that) index.
 */
function ShowType (sTypeName, nHistoryIndex)
{
	if (nHistoryIndex == -1)
	{
		if (CurrentTypeName != "")
		{
			TypeHistory.push(CurrentTypeName);
			ShowHistory();
		}
	}
	else
	{
		TypeHistory = TypeHistory.slice(0,nHistoryIndex);
		ShowHistory();
	}
	CurrentTypeName = sTypeName;

	var aElement = document.getElementById('result');

    // Remove the old content.
    while(aElement.childNodes.length > 0)
    	aElement.removeChild(aElement.firstChild);

    // Create the new content.
    var list = CreateDomTreeForType(GetTopLevelNodeForTypeName(sTypeName), "ul");
    
    // Show the new content.
    aElement.appendChild(list);
}




/** Create a dom sub tree for the given OOXML type that is ready for insertion into the global DOM tree.
 */
function CreateDomTreeForType (aNode, sType)
{
    var aEntry = document.createElement(sType);

    if (typeof aNode==='undefined')
    {
    	aEntry.innerHTML = "undefined node";
    }
    else if (typeof aNode.type==='undefined')
    {
    	aEntry.innerHTML = "unknown type";
    }
    else
    {
	    switch(aNode.type)
	    {
	    	case "attribute":
	    		aEntry.innerHTML = CreateValueTable([
	    				"attribute",
	    				"type:", GetTypeLink(aNode["value-type"], aNode["value-type"], -1),
	    				"use:", aNode.use]);
	        	break;
	    			
            case "attribute-reference":
                aEntry.innerHTML = CreateReference(aNode["referenced-attribute"]);
                break;
    
	    	case "builtin":
	    		aEntry.innerHTML = CreateValueTable(
	    		        ["builtin",
	    		         "name:", aNode['builtin-type']
	    		        ]);
	    		break;
	    		
            case "complex-type":
                aEntry.innerHTML = aNode.type + " " + aNode.name;
                break;
    
            case "complex-type-reference":
                aEntry.innerHTML = CreateReference(aNode["referenced-complex-type"]);
                break;
    
	        case "group":
	        	aEntry.innerHTML = aNode.type + " " + aNode.name;
	        	break;
	
            case "group-reference":
                aEntry.innerHTML = CreateReference("group", aNode["referenced-group"]);
                break;
    
	        case "element":
	        	aEntry.innerHTML = "element <b>" + aNode["tag"] + "</b> -> " + GetTypeLink(aNode["result-type"], aNode["result-type"], -1);
	        	break;
	
            case "occurrence":
                aEntry.innerHTML = aNode.minimum +" -> " + aNode.maximum;
                break;
    
            case "restriction":
                aEntry.innerHTML = CreateRestrictionRepresentation(aNode);
                break;
    
            case "sequence":
                aEntry.innerHTML = "sequence";
                break;
    
            case "simple-type":
                aEntry.innerHTML = aNode.type + " " + aNode.name;
                break;
    
            case "simple-type-reference":
                aEntry.innerHTML = CreateReference("simple-type", aNode["referenced-simple-type"]);
                break;
    
	        default:
	        	aEntry.innerHTML = aNode.type;
		    	break;
	    }
	
	    // Add nodes for attributes.
	    var aAttributes= aNode["attributes"];
	    if ( ! (typeof aAttributes==='undefined' || aAttributes.length == 0))
	    {
			var aAttributeList = document.createElement("ul");
			aEntry.appendChild(aAttributeList);
		
			for (var nIndex=0; nIndex<aAttributes.length; ++nIndex)
			{
			    var aAttributeEntry = CreateDomTreeForType(aAttributes[nIndex], "li");
			    aAttributeList.appendChild(aAttributeEntry);
			}
	    }

	    // Add nodes for children.
	    var aChildren = aNode["children"];
	    if ( ! (typeof aChildren==='undefined' || aChildren.length == 0))
	    {
			var aChildrenList = document.createElement("ul");
			aEntry.appendChild(aChildrenList);
		
			for (var nIndex=0; nIndex<aChildren.length; ++nIndex)
			{
			    var aChildrenEntry = CreateDomTreeForType(aChildren[nIndex], "li");
			    aChildrenList.appendChild(aChildrenEntry);
			}
	    }
    }
    return aEntry;
}




function GetTypeLink (sText, sTarget, nIndex)
{
    return "<span class=\"typelink\" id=\""+sTarget+"\" onclick=\"ShowType('"+sTarget+"',"+nIndex+")\">"+sText+"</span>";
}




function CreateValueTable (aValues)
{
	var sResult = "<table class=\"value-table\"><tr><td>"+aValues[0]+"</td><td>"+aValues[1]+"</td><td>"+aValues[2]+"</td></tr>";
	for (nIndex=3; nIndex<aValues.length; nIndex+=2)
	{
		sResult += "<tr><td></td><td>"+aValues[nIndex]+"</td><td>"+aValues[nIndex+1]+"</td></tr>";
	}
	sResult += "</table>";
	return sResult;
}




function CreateReference (sWhat, sTypeName)
{
    return "reference to "+sWhat+" "+GetTypeLink(sTypeName, sTypeName, -1) + " " 
        +CreateButton(
            sTypeName,
            "show",
            "ToggleTypeReferenceExpansion('"+sTypeName+"')")
            +"<br><span id=\"expansion-"+sTypeName+"\"></span>";
}




function CreateButton (sId, sText, sExpandAction)
{
    return "<span class=\"button\" id=\"button-"+sId+"\" onClick=\""+sExpandAction+"\">"+sText+"</span>";
}




function ToggleTypeReferenceExpansion(sTypeName)
{
    var aButton = document.getElementById("button-"+sTypeName);
    var aExpansion = document.getElementById("expansion-"+sTypeName);
    if (aButton.innerHTML == "show")
    {
        aExpansion.appendChild(CreateDomTreeForType(Data[sTypeName], "span"));
        aButton.innerHTML = "hide";
    }
    else
    {
        aExpansion.innerHTML = "";
        aButton.innerHTML = "show";
    }
}




function ShowHistory ()
{
	var aElement = document.getElementById('history');
	var sContent = "History:";
	for (nIndex=0; nIndex<TypeHistory.length; ++nIndex)
	{
		if (nIndex == 0)
			sContent += " ";
		else
			sContent += ", ";
		sContent += GetTypeLink(TypeHistory[nIndex], TypeHistory[nIndex], nIndex);
	}
	aElement.innerHTML = sContent;
}




function CreateRestrictionRepresentation (aNode)
{
    var aTableData = ["restriction", "based on:", GetTypeLink(aNode['base-type'], aNode['base-type'], -1)];
    AddValue(aNode, "enumeration", aTableData);
    AddValue(aNode, "pattern", aTableData);
    AddValue(aNode, "length", aTableData);
    return CreateValueTable(aTableData);
}



function AddValue (aMap, sKey, aTableData)
{
    if (sKey in aMap)
    {
        aTableData.push(sKey+":");
        aTableData.push(aMap[sKey]);
    }
}
