DBFilter Readme
===============

The DocBook filter (DBFilter) is essentially teh XSLTFilter that is 
delivered with Openoffice, except that it contains an extra routine
for Masking DOCTYPE Entity declarations. In this way, Entities 
which are referenced inside the XML Document to be imported, 
are converted into an <entity name="ent_name"> tag. In this way,
the entities can be imported into OOo as "set" and "get" variables.

On Export, these variables can be written out once again as correct
Enity decls and Entity references. 

 