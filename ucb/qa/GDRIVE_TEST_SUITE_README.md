# Google Drive Integration Test Suite

This directory contains comprehensive tests for the Google Drive REST API integration in LibreOffice.

## Test Files Overview

### Core Unit Tests
- **`test_gdrive_json.cxx`** - Tests JSON parsing and generation utilities
- **`test_gdrive_api_client.cxx`** - Tests the main API client with mock responses  
- **`test_gdrive_provider.cxx`** - Tests the UCB provider functionality

### Integration Tests
- **`test_gdrive_integration.cxx`** - Full workflow integration tests
- **`test_gdrive_mock_server.cxx`** - Mock HTTP server for realistic testing

### Advanced Tests
- **`test_gdrive_performance.cxx`** - Performance and load testing for large folders
- **`test_gdrive_complete.cxx`** - Complete end-to-end scenarios
- **`test_gdrive_content.cxx`** - UCB Content implementation tests

### Test Utilities
- **`gdrive_test_data.hxx`** - Shared test data, mock responses, and utilities

## Running the Tests

### Prerequisites
- LibreOffice development environment set up
- `ENABLE_GDRIVE=TRUE` in configure
- Boost and cURL libraries available

### Build and Run
```bash
# Build the test suite
make CppunitTest_ucb_gdrive

# Run all tests
make CppunitTest_ucb_gdrive.run

# Run with verbose output
make CppunitTest_ucb_gdrive.run CPPUNIT_ARGS="--verbose"
```

### Individual Test Execution
The test suite is designed to run without requiring actual Google Drive credentials or network connectivity. All tests use mock responses and simulated network behavior.

## Test Categories

### 1. JSON Parsing Tests (`test_gdrive_json.cxx`)
- **Purpose**: Verify correct parsing of Google Drive API JSON responses
- **Coverage**: 
  - File and folder listings
  - Pagination tokens
  - Error responses
  - DateTime parsing
  - Metadata creation

**Key Tests:**
- `testParseFolderListingEmpty()` - Empty folder handling
- `testParseFolderListingWithPagination()` - Large folder pagination
- `testParseDateTime()` - ISO 8601 date parsing
- `testCreateCopyMetadata()` - File copy request generation

### 2. API Client Tests (`test_gdrive_api_client.cxx`)
- **Purpose**: Test the GoogleDriveApiClient class functionality
- **Coverage**:
  - Basic CRUD operations
  - Error handling
  - Network failure simulation
  - Retry mechanisms

**Key Tests:**
- `testListFolderSuccess()` - Successful folder listing
- `testGetFileInfoNotFound()` - 404 error handling
- `testRetryMechanism()` - Exponential backoff testing

### 3. Provider Tests (`test_gdrive_provider.cxx`)
- **Purpose**: Test UCB provider integration
- **Coverage**:
  - URL validation and parsing
  - Content provider creation
  - Service registration

**Key Tests:**
- `testIsGDriveURL()` - URL scheme validation
- `testGetFileIdFromURL()` - File ID extraction
- `testProviderCreation()` - UCB provider instantiation

### 4. Integration Tests (`test_gdrive_integration.cxx`)
- **Purpose**: Full workflow testing with realistic scenarios
- **Coverage**:
  - Complete user workflows
  - Multi-step operations
  - Error recovery
  - Large folder handling

**Key Tests:**
- `testCompleteWorkflow()` - End-to-end file operations
- `testPaginatedFolderListing()` - Large folder pagination
- `testErrorHandling()` - Various error scenarios

### 5. Performance Tests (`test_gdrive_performance.cxx`)
- **Purpose**: Validate performance characteristics and scalability
- **Coverage**:
  - Large folder performance
  - Memory usage
  - Concurrent operations
  - Network latency handling

**Key Tests:**
- `testLargeFolderListing()` - 1000+ file folders
- `testConcurrentOperations()` - Multi-threaded access
- `testMemoryUsage()` - Memory leak detection

## Mock Data and Responses

The test suite uses comprehensive mock data defined in `gdrive_test_data.hxx`:

### Standard Test Files
- Documents folder with various file types
- Office documents (Word, Excel, PowerPoint)
- Plain text and other common formats
- Large folders for pagination testing

### Mock Responses
- OAuth token responses
- File listing with metadata
- Error responses (404, 401, 429, 500)
- Pagination responses with nextPageToken

### Test Scenarios
- Network delays and timeouts
- Rate limiting (429 responses)
- Authentication failures
- Server errors with retry logic

## Performance Benchmarks

The performance tests establish baseline expectations:
- **Small folders** (≤10 files): <100ms
- **Medium folders** (≤100 files): <500ms  
- **Large folders** (≤1000 files): <2000ms
- **Pagination**: <200ms average per page
- **Memory usage**: <10MB for 1000 files

## Error Handling Validation

Tests verify proper handling of:
- **HTTP 404**: File not found
- **HTTP 401**: Authentication failure
- **HTTP 403**: Insufficient permissions
- **HTTP 429**: Rate limiting
- **HTTP 500**: Server errors
- **Network timeouts**: Connection failures

## Test Data Management

### Cleanup
Tests are designed to be self-contained and don't require cleanup of external resources.

### Isolation
Each test uses unique mock data to prevent interference between tests.

### Repeatability
All tests use deterministic mock responses for consistent results.

## Debugging Tests

### Verbose Output
```bash
make CppunitTest_ucb_gdrive.run CPPUNIT_ARGS="--verbose"
```

### Single Test Execution
Individual test methods can be run using CppUnit's test selection:
```bash
# Run only JSON parsing tests
make CppunitTest_ucb_gdrive.run CPPUNIT_ARGS="--test-path=GDriveJsonTest"
```

### Debug Builds
Enable debug output in the test code:
```cpp
#ifdef DEBUG
printf("Debug: Processing file %s\n", fileName.getStr());
#endif
```

## Extending the Test Suite

### Adding New Tests
1. Add test methods to existing test classes
2. Use `gdrive_test_data.hxx` utilities for consistent mock data
3. Follow naming convention: `test{Operation}{Scenario}()`
4. Update this README with test descriptions

### Adding Mock Responses
1. Add new responses to `gdrive_test_data.hxx`
2. Use realistic Google Drive API response formats
3. Include both success and error scenarios
4. Test edge cases and boundary conditions

### Performance Testing
1. Add new scenarios to `test_gdrive_performance.cxx`
2. Set realistic performance expectations
3. Test with various data sizes
4. Monitor memory usage and network efficiency

## Troubleshooting

### Common Issues
- **Build failures**: Ensure `ENABLE_GDRIVE=TRUE` in configure
- **Missing dependencies**: Verify Boost and cURL are available
- **Test failures**: Check mock response formats match API changes

### Platform Differences
Tests are designed to work across platforms, but timing-sensitive tests may need adjustment for slower systems.

### CI/CD Integration
Tests are suitable for continuous integration as they don't require external dependencies or credentials.

## Contributing

When adding new Google Drive functionality:
1. Add corresponding unit tests
2. Update integration tests for new workflows
3. Add performance tests for operations involving large data
4. Update mock responses to match real API behavior
5. Document new test scenarios in this README

## API Documentation References

- [Google Drive API v3](https://developers.google.com/drive/api/v3/reference)
- [OAuth 2.0 for Web Server Applications](https://developers.google.com/identity/protocols/oauth2/web-server)
- [LibreOffice UCB Documentation](https://wiki.documentfoundation.org/Development/UCB)