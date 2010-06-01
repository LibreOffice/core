#
# # ------------------------------------------------------------------------------
#
# sub DB_INSERT_INTO_TABLE_STATUS()
# {
#     # my $sDocID = shift;
#     # my $sDBDistinct = shift;
#
#     my $sHostname = hostname;
#
#     my $sSQL = "INSERT INTO status (docid, dbdistinct2, hostname)";
#     $sSQL .= "  VALUES ($docid, '$dbdistinct', '$sHostname')";
#     ExecSQL($sSQL);
# }
# sub DB_UPDATE_TABLE_STATUS_SET_INFO($)
# {
#     # my $sDocID = shift;
#     # my $sDBDistinct = shift;
#     my $sInfo = shift;
#
#     # my $sHostname = hostname;
#
#     my $sInsertSQL = "UPDATE status SET info='$sInfo' WHERE docid=$docid AND dbdistinct2='$dbdistinct'";
#     ExecSQL($sInsertSQL);
# }
#
# sub DB_UPDATE_TABLE_DOCUMENTS_SET_STATE_INFO($$)
# {
#     # my $sDocID = shift;
#     my $sStatus = shift;
#     my $sError = shift;
#
#     my $sSQL = "UPDATE documents";
#     $sSQL .= " SET state='" . $sStatus . "'";
#     $sSQL .= ",info='" . $sError . "'";
#     $sSQL .= " WHERE docid=$docid";
#     ExecSQL($sSQL);
# }
# sub DB_UPDATE_TABLE_STATUS_SET_STATE($)
# {
#     # my $sDocID = shift;
#     my $sStatus = shift;
#
#     my $sSQL = "UPDATE status";
#     $sSQL .= " SET state='" . $sStatus . "'";
#     $sSQL .= " WHERE docid=$docid";
#     ExecSQL($sSQL);
# }
#
# # sub DB_UPDATE_TABLE_STATUS_SET_STATE_FAILED()
# # {
# #     DB_UPDATE_TABLE_STATUS_SET_STATE("FAILED-FAILED");
# # }
# # ------------------------------------------------------------------------------
# # sub getDBConnectionString()
# # {
# #     # return "server:jakobus,db:jobs_convwatch,user:admin,passwd:admin";
# #     return "server:unoapi,db:jobs_convwatch,user:convwatch,passwd:convwatch";
# # }
# # ------------------------------------------------------------------------------
# sub getSourceInfo($)
# {
#     my $sDBStr = shift;
#
#     my $sSourceVersion;
#     if ( $sDBStr =~ / sourceversion='(.*?)',/ )
#     {
#         $sSourceVersion = $1;
#         log_print( "sSourceVersion: $sSourceVersion\n");
#     }
#     if (! $sSourceVersion)
#     {
#         log_print( "Error: no value for sourceversion found.\n");
#         return;
#     }
#     my $sSourceName;
#     if ( $sDBStr =~ / sourcename='(.*?)',/ )
#     {
#         $sSourceName = $1;
#         log_print( "sSourceName: $sSourceName\n");
#     }
#     my $sSourceCreatorType;
#     if ( $sDBStr =~ / sourcecreatortype='(.*?)',/ )
#     {
#         $sSourceCreatorType = $1;
#         log_print( "sSourceCreatorType: $sSourceCreatorType\n");
#     }
#     return $sSourceVersion, $sSourceName, $sSourceCreatorType;
# }
# # ------------------------------------------------------------------------------
# sub getDestinationInfo($)
# {
#     my $sDBStr = shift;
#
#     my $sDestinationVersion;
#     if ( $sDBStr =~ / destinationversion='(.*?)',/ )
#     {
#         $sDestinationVersion = $1;
#         log_print( "sDestinationVersion: $sDestinationVersion\n");
#     }
#     if (! $sDestinationVersion)
#     {
#         log_print( "Error: no value for destinationversion found.\n");
#         return;
#     }
#     my $sDestinationName;
#     if ( $sDBStr =~ / destinationname='(.*?)',/ )
#     {
#         $sDestinationName = $1;
#         log_print( "sDestinationName: $sDestinationName\n");
#     }
#     my $sDestinationCreatorType;
#     if ( $sDBStr =~ / destinationcreatortype='(.*?)',/ )
#     {
#         $sDestinationCreatorType = $1;
#         log_print( "sDestinationCreatorType: $sDestinationCreatorType\n");
#     }
#     return $sDestinationVersion, $sDestinationName, $sDestinationCreatorType;
# }
# # ------------------------------------------------------------------------------
# # sub getMailAddress($)
# # {
# #     my $sDBStr = shift;
# #     my $sMailAddress = "";
# #     if ( $sDBStr =~ / mailfeedback='(.*?)',/ )
# #     {
# #         $sMailAddress = $1;
# #         log_print( "sMailAddress: $sMailAddress\n");
# #     }
# #     return $sMailAddress;
# # }
#
# # sub getDocumentInfo($)
# # {
# #     my $sDBStr = shift;
# #
# #     my $sDocumentPoolPath;
# #     if ( $sDBStr =~ / documentpoolpath='(.*?)',/ )
# #     {
# #         $sDocumentPoolPath = $1;
# #         log_print( "sDocumentPoolPath: $sDocumentPoolPath\n");
# #     }
# #     if (! $sDocumentPoolPath)
# #     {
# #         log_print( "Error: no value for documentpoolpath found.\n");
# #         return;
# #     }
# #     my $sDocumentPool;
# #     if ( $sDBStr =~ / documentpool='(.*?)',/ )
# #     {
# #         $sDocumentPool = $1;
# #         log_print( "sDocumentPool: $sDocumentPool\n");
# #     }
# #     if (! $sDocumentPool)
# #     {
# #         log_print( "Error: no value for documentpool found.\n");
# #         return;
# #     }
# #     my $sDocumentName;
# #     if ( $sDBStr =~ / name='(.*?)',/ )
# #     {
# #         $sDocumentName = $1;
# #         log_print( "sDocumentName: $sDocumentName\n");
# #     }
# #     return $sDocumentPoolPath, $sDocumentPool, $sDocumentName;
# # }
#
# sub getDistinct($)
# {
#     my $sDBStr = shift;
#     my $sDBDistinct;
#     if ( $sDBStr =~ / dbdistinct2='(\S*?)',/ )
#     {
#         $sDBDistinct = $1;
#         log_print( "dbdistinct2: $sDBDistinct\n");
#     }
#     return $sDBDistinct;
# }
#
# sub getIDInfo($)
# {
#     my $sDBStr = shift;
#     # my $dbdistinct;
#
#     my $sDBDistinct = getDistinct($sDBStr);
#     # if ( $sDBStr =~ / dbdistinct2='(\S*?)',/ )
#     # {
#     #     $sDBDistinct = $1;
#     #     log_print( "dbdistinct2: $sDBDistinct\n");
#     # }
#     if (! $sDBDistinct)
#     {
#         log_print( "Error: no dbdistinct given.\n");
#         return;
#     }
#     my $sDocID;
#     if ( $sDBStr =~ / docid=(\S*?),/ )
#     {
#         $sDocID = $1;
#         log_print( "docid: $sDocID\n");
#     }
#     if (! $sDocID)
#     {
#         log_print( "Error: no docid given.\n");
#         return;
#     }
#     return $sDBDistinct, $sDocID;
# }
#
