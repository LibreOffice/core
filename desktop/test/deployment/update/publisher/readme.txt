--The folder contains extensions which use in the description.xml the following:

-The <publisher> element
-The <release-notes> element

Both element contain localized child elements.

The following table shows what localized item is used, when the Office the locale 
en-US uses.


Localization:

Installed office en-US
         | publisher     | release notes
=============================================
pub1.oxt | en-US         | en-US
--------------------------------------------- 
pub2.oxt | en-US-region1 | en-US-region1
---------------------------------------------
pub3.oxt | en            | en
---------------------------------------------
pub4.oxt | en-GB         | en-GB
---------------------------------------------
pub5.oxt | de            | de


================================================================================
pub6.oxt
================================================================================
like pub1 but without release notes.


================================================================================
pub7.oxt
================================================================================
like pub1 but without publisher name.

================================================================================
pub8.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop.

pub8.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

pub8.oxt provides <publisher> and <release-notes>. These information should be transferred
in the update feed and not those entered in the repository.

Test
----
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. This setting should be ignored
 when generating the udpate feed. Instead the publisher name from the extension is used.
 Enter "Publisher Title" : some arbitrary company
 "Publisher URL": any arbitrary URL but not:
 http://extensions.openoffice.org/testarea/desktop/publisher/publisher_en-US.html
  
-Create a new release for the extension. Upload the version 2.0 (update/pub8.oxt). 
 Provide release notes. These release notes should later not be seen when clicking on 
 the release notes link.


Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/publisher/pub8.oxt

-Run the update in the Extension Manager


Result:
The Update Dialog should show the publisher name as provided in the description.xml. For example,
when lang=en-US was selected: My OpenOffice en-US

A release notes link is displayed with an URL to the release notes as provided in 
the description.xml. For example, when lang=en-US was selected:
"http://extensions.openoffice.org/testarea/desktop/publisher/publisher_en-US.html


================================================================================
pub9.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop.

pub9.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

pub9.oxt provides <publisher>. That means the update feed should
contain the <publisher> as provided by the extension and the release notes as entered
in the repository.

Test
----
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. This setting should be ignored
 when generating the udpate feed. Instead the publisher name from the extension is used.
 Enter "Publisher Title" : some arbitrary company name
 "Publisher URL": any arbitrary URL but not:
 http://extensions.openoffice.org/testarea/desktop/publisher/publisher_en-US.html
  
-Create a new release for the extension. Upload the version 2.0 (update/pub9.oxt). 
 Provide release notes. These release notes should later be displayed when clicking on 
 the release notes link.


Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/publisher/pub9.oxt

-Run the update in the Extension Manager


Result:
The Update Dialog should show the publisher name as provided in the description.xml. For example,
when lang=en-US was selected: My OpenOffice en-US

A release notes link is displayed with an URL to the release notes as provided in the release notes
field on the edit page for the extension in the repository.

================================================================================
pub10.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop.

pub10.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

pub10.oxt provides <release-notes>. That means the update feed should
contain the <release-notes> as provided by the extension and the publisher name/URLs as entered
in the repository.

Test
----
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. They should then be in the udpate 
 feed. 
  
-Create a new release for the extension. Upload the version 2.0 (update/pub10.oxt). 
 Provide release notes. These release notes should NOT  be displayed when clicking on 
 the release notes link. Instead the release notes provided by pub10.oxt should be displayed.


Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/publisher/pub10.oxt

-Run the update in the Extension Manager


Result:
The Update Dialog should show the publisher name as provided in the repository.

A release notes link is displayed with an URL to the release notes as provided in the 
pub10.oxt.  For example, when the locale of the office is en-US then this page will be 
displayed:
For example,
when lang=en-US was selected: My OpenOffice en-US

================================================================================
pub11.oxt
================================================================================
Need not be committed in extensions/www/testarea/desktop.

pub11.oxt is intended for tests with extensions.services.openoffice.org.
It does not contain <update-information>. That is, the Extension Manager will obtain
the update information from the repository as feed:

http://updateext.services.openoffice.org/ProductUpdateService/check.Update

pub10.oxt neither provides <release-notes> nor <publisher>. That means the update feed should
contain these data as provided by the user on the repository web site.

Test
----
Repository:

-Create the new extension in the repository. 
-Provide a company name and a URL to the company website. They should then be in the udpate 
 feed. 
  
-Create a new release for the extension. Upload the version 2.0 (update/pub11.oxt). 
 Provide release notes. These release notes should be displayed when clicking on 
 the release notes link. 


Office:
-Install version 1.0 of the extension:
 desktop/test/deployment/update/publisher/pub11.oxt

-Run the update in the Extension Manager


Result:
The Update Dialog should show the publisher name as provided in the repository.

A release notes link is displayed which leads to the release notes kept in the repository.

