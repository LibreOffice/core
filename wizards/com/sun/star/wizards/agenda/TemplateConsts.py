FILLIN_TITLE = "<title>"
FILLIN_DATE = "<date>"
FILLIN_TIME = "<time>"
FILLIN_LOCATION = "<location>"
'''
section name <b>prefix</b> for sections that contain items.
this is also used as table name prefix, since each items section
must contain a table whos name is identical name to the section's name.
'''
SECTION_ITEMS = "AGENDA_ITEMS"
'''
the name of the section which contains the topics.
'''
SECTION_TOPICS = "AGENDA_TOPICS"
'''
the name of the parent minutes section.
'''
SECTION_MINUTES_ALL = "MINUTES_ALL"
'''
the name of the child nimutes section.
This section will be duplicated for each topic.
'''
SECTION_MINUTES = "MINUTES"
'''
taged headings and names.
These will be searched in item tables (in the template) and will be
replaced with resource strings.

headings...
'''
FILLIN_MEETING_TYPE = "<meeting-type>"
FILLIN_BRING = "<bring>"
FILLIN_READ = "<read>"
FILLIN_NOTES = "<notes>"
'''
names...
'''
FILLIN_CALLED_BY = "<called-by>"
FILLIN_FACILITATOR = "<facilitator>"
FILLIN_PARTICIPANTS = "<attendees>"
FILLIN_NOTETAKER = "<notetaker>"
FILLIN_TIMEKEEPER = "<timekeeper>"
FILLIN_OBSERVERS = "<observers>"
FILLIN_RESOURCE_PERSONS = "<resource-persons>"

'''
Fillins for the topic table.
These strings will be searched inside the topic table as
part of detecting its structure.
'''
FILLIN_TOPIC_NUMBER = "<num>"
FILLIN_TOPIC_TOPIC = "<topic>"
FILLIN_TOPIC_RESPONSIBLE = "<responsible>"
FILLIN_TOPIC_TIME = "<topic-time>"
'''
fillins for minutes.
These will be searched in the minutes section and will be replaced
with the appropriate data.
'''
FILLIN_MINUTES_TITLE = "<minutes-title>"
FILLIN_MINUTES_LOCATION = "<minutes-location>"
FILLIN_MINUTES_DATE = "<minutes-date>"
FILLIN_MINUTES_TIME = "<minutes-time>"
'''
Minutes-topic fillins
These will be searched in the minutes-child-section, and
will be replaced with topic data.
'''
FILLIN_MINUTE_NUM = "<mnum>"
FILLIN_MINUTE_TOPIC = "<mtopic>"
FILLIN_MINUTE_RESPONSIBLE = "<mresponsible>"
FILLIN_MINUTE_TIME = "<mtime>"
