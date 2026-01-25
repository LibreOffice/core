#ifndef INCLUDED_OFFICELABS_AGENTCONNECTION_HXX
#define INCLUDED_OFFICELABS_AGENTCONNECTION_HXX

#include <officelabs/officelabsdllapi.h>
#include <rtl/ustring.hxx>
#include <string>
#include <vector>
#include <optional>

namespace officelabs {

// Command for automatic document editing
struct OFFICELABS_DLLPUBLIC AutoEditCommand {
    OUString action;     // "insert", "replace", "delete", "format", "clear_and_write", "paragraph_format", "create_list", "insert_page_break", "apply_style", "undo", "redo"
    OUString findText;   // Text to find for replace/delete/format
    OUString newText;    // Text to insert or replace with
    OUString position;   // "cursor", "end", "start", "after_find", "before_find"
    bool asParagraph = false;  // If true, insert as new paragraph (with line break)
    // Character formatting
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
    bool superscript = false;
    bool subscript = false;
    int headingLevel = 0;  // 0 = normal, 1-6 = heading levels
    OUString fontColor;  // Hex color like "#FF0000" for red, empty = no change
    OUString highlightColor;  // Background highlight color
    OUString fontName;   // Font family name
    double fontSize = 0;  // Font size in points, 0 = no change
    // Paragraph formatting
    OUString alignment;  // "left", "center", "right", "justify"
    double lineSpacing = 0;  // 1.0 = single, 1.5, 2.0 = double
    double spaceBefore = 0;  // Space before paragraph in points
    double spaceAfter = 0;   // Space after paragraph in points
    double indentLeft = 0;   // Left indent in cm
    double indentRight = 0;  // Right indent in cm
    double indentFirstLine = 0;  // First line indent in cm
    // List creation
    OUString listType;   // "bullet", "numbered", "letter", "roman"
    std::vector<OUString> listItems;  // Items for the list
    // Table creation
    int tableRows = 0;
    int tableColumns = 0;
    std::vector<std::vector<OUString>> tableData;  // 2D array of cell contents
    bool headerRow = true;
    // Search & Replace
    bool caseSensitive = true;
    bool wholeWords = false;
    // Style application
    OUString styleName;  // Named style to apply
    // Undo/Redo
    int undoSteps = 1;   // Number of steps to undo/redo
};

struct OFFICELABS_DLLPUBLIC AgentResponse {
    OUString message;
    bool hasPatch;
    OUString patchType;
    OUString patchTarget;
    OUString patchOldValue;
    OUString patchNewValue;
    OUString patchDiff;
    std::vector<AutoEditCommand> autoEdits;  // Commands to execute automatically
};

class OFFICELABS_DLLPUBLIC AgentConnection {
private:
    std::string m_backendUrl;
    bool m_connected;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    AgentResponse parseResponse(const std::string& json);

public:
    AgentConnection();
    ~AgentConnection();
    
    bool checkConnection();
    bool isConnected() const { return m_connected; }
    
    AgentResponse sendMessage(const OUString& message, const OUString& documentContent, const OUString& selection = OUString());
    void setBackendUrl(const OUString& url);
};

} // namespace officelabs

#endif
