#ifndef INCLUDED_OFFICELABS_AGENTCONNECTION_HXX
#define INCLUDED_OFFICELABS_AGENTCONNECTION_HXX

#include <officelabs/officelabsdllapi.h>
#include <rtl/ustring.hxx>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>

namespace officelabs {

// Streaming event types (like Claude for Excel real-time updates)
enum class StreamEventType {
    SESSION,          // Session started
    ITERATION,        // New agent iteration
    TOOL_START,       // Tool call started
    TOOL_COMPLETE,    // Tool call completed
    TEXT,             // Text chunk received
    DONE,             // Processing complete
    STREAM_ERROR      // Error occurred (named to avoid Windows ERROR macro conflict)
};

// Event from SSE stream
struct StreamEvent {
    StreamEventType type;
    OUString toolName;      // For TOOL_START/TOOL_COMPLETE
    OUString toolId;        // Tool call ID
    OUString toolResult;    // Result for TOOL_COMPLETE
    OUString text;          // For TEXT events
    OUString error;         // For ERROR events
    int iteration = 0;      // For ITERATION events
};

// Callback for streaming events
using StreamCallback = std::function<void(const StreamEvent&)>;

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

// Clarification question for complex tasks
struct ClarificationQuestion {
    OUString id;
    OUString question;
    std::vector<OUString> options;  // Empty if free text
    OUString defaultValue;

    // Inline definitions to avoid DLL export issues
    ClarificationQuestion() = default;
    ClarificationQuestion(const ClarificationQuestion&) = default;
    ClarificationQuestion& operator=(const ClarificationQuestion&) = default;
    ~ClarificationQuestion() = default;
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
    // Clarification flow
    bool needsClarification = false;
    std::vector<ClarificationQuestion> clarificationQuestions;
};

class OFFICELABS_DLLPUBLIC AgentConnection {
private:
    std::string m_backendUrl;
    bool m_connected;
    bool m_cancelRequested;  // For cancelling streaming requests

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t StreamWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    AgentResponse parseResponse(const std::string& json);
    StreamEvent parseSSEEvent(const std::string& eventData);

public:
    AgentConnection();
    ~AgentConnection();

    bool checkConnection();
    bool isConnected() const { return m_connected; }

    // Standard synchronous request
    AgentResponse sendMessage(const OUString& message, const OUString& documentContent, const OUString& selection = OUString());
    AgentResponse sendMessageWithClarification(const OUString& message, const OUString& documentContent, const OUString& selection, const std::map<OUString, OUString>& clarificationAnswers);

    // Streaming request with real-time callbacks (Claude for Excel style)
    AgentResponse sendMessageStream(const OUString& message, const OUString& documentContent, const OUString& selection, StreamCallback callback);

    // Cancel ongoing streaming request
    void cancelStream();

    void setBackendUrl(const OUString& url);
};

} // namespace officelabs

#endif
